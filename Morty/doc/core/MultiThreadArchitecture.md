# Morty 引擎多线程架构

## 概述

Morty 引擎实现了完整的多线程架构，包括线程池、任务图系统、异步资源加载以及主线程/渲染线程分离等核心功能。

---

## 1. 线程池 (MThreadPool)

### 文件位置
- `Core/Thread/MThreadPool.h`
- `Core/Thread/MThreadPool.cpp`

### 架构设计

```
┌─────────────────────────────────────────────────────────┐
│                     MThreadPool                          │
├─────────────────────────────────────────────────────────┤
│  Thread[0] - Main Thread (EMainThread)                  │
│  Thread[1] - Render Thread (ERenderThread)              │
│  Thread[2-9] - Worker Threads (EAny)                    │
├─────────────────────────────────────────────────────────┤
│  m_waitingWork         - 通用工作队列                    │
│  m_specificWaitingWork - 特定线程队列(按线程索引)         │
│  m_ConditionVariable   - 线程唤醒通知                    │
└─────────────────────────────────────────────────────────┘
```

### 线程类型定义

```cpp
enum class METhreadType : int
{
    EAny           = -2,    // 任意线程执行
    ECurrentThread = -1,    // 当前线程执行
    EMainThread    = 0,     // 主线程 (索引0)
    ERenderThread  = 1,     // 渲染线程 (索引1)
    ENameThreadNum          // 命名线程数量
};
```

### 核心数据结构

```cpp
class MThreadPool
{
    std::mutex                                                     m_ConditionMutex;
    std::condition_variable                                        m_ConditionVariable;
    std::array<std::thread, MGlobal::M_MAX_THREAD_NUM>             m_thread;
    static std::array<METhreadType, MGlobal::M_MAX_THREAD_NUM>     s_tThreadType;

    std::queue<MThreadWork>                                        m_waitingWork;
    std::array<std::queue<MThreadWork>, MGlobal::M_MAX_THREAD_NUM> m_specificWaitingWork;

    bool m_initialized = false;
    bool m_close       = false;
    size_t m_closeThreadCount = 0;
};
```

### 关键方法

| 方法 | 功能 |
|------|------|
| `Initialize()` | 初始化线程池，创建工作线程 |
| `AddWork(const MThreadWork& work)` | 添加工作到队列 |
| `ThreadRun()` | 工作线程主循环 |
| `GetCurrentThreadType()` | 获取当前线程类型 |
| `GetCurrentThreadIndex()` | 获取当前线程索引 |

### 工作调度流程

1. **ECurrentThread**: 立即在当前线程执行
2. **当前线程类型匹配**: 立即执行
3. **EAny**: 添加到通用工作队列
4. **指定线程类型**: 添加到特定线程队列
5. 通过条件变量通知等待的工作线程

### 配置选项

- **最大线程数**: `M_MAX_THREAD_NUM = 10`（定义于 `MGlobal.h`）
- **单线程模式**: `bSingleThreadMode = true`（可切换）

---

## 2. 任务/工作系统 (MThreadWork)

### 文件位置
- `Core/Thread/MThreadWork.h`

### 结构定义

```cpp
struct MORTY_API MThreadWork {
    std::function<void(void)> funcWorkFunction = nullptr;  // 执行的函数
    int eThreadType = static_cast<int>(METhreadType::EAny); // 目标线程类型
};
```

### 特点

- 轻量级工作包装
- 使用 `std::function` 存储任意可调用对象
- 支持按线程类型调度
- 绑定函数使用宏: `M_CLASS_FUNCTION_BIND_1_0`, `M_CLASS_FUNCTION_BIND_0_1` 等

---

## 3. 任务图系统 (Task Graph)

### 文件位置
- `Core/TaskGraph/MTaskGraph.h/.cpp`
- `Core/TaskGraph/MTaskNode.h/.cpp`
- `Core/TaskGraph/MTaskGraphWalker.h`（接口）
- `Core/TaskGraph/MMultiThreadTaskGraphWalker.h/.cpp`
- `Core/TaskGraph/MSingleThreadTaskGraphWalker.h/.cpp`

### 架构设计

任务图是引擎的核心调度机制，允许定义任务节点的依赖关系并按顺序或并行执行。

```
┌──────────┐     ┌──────────┐     ┌──────────┐
│ TaskNode │────▶│ TaskNode │────▶│ TaskNode │
│ (Start)  │     │ (Middle) │     │  (End)   │
└──────────┘     └────┬─────┘     └──────────┘
                      │                 ▲
                      ▼                 │
                ┌──────────┐           │
                │ TaskNode │───────────┘
                │ (Branch) │
                └──────────┘
```

### 任务节点 (MTaskNode)

```cpp
class MTaskNode
{
    MStringId m_strNodeName;
    MTaskGraph* m_graph;
    size_t m_priorityLevel;     // 编译后计算的优先级
    size_t m_id;
    METhreadType m_threadType;  // 执行线程类型

    std::vector<MTaskNodeInput*> m_input;    // 输入依赖
    std::vector<MTaskNodeOutput*> m_output;  // 输出依赖
    std::function<void(MTaskNode*)> m_funcTaskFunction; // 执行函数
};
```

### 任务图 (MTaskGraph)

```cpp
class MTaskGraph
{
    std::unordered_map<size_t, MTaskNode*> m_taskNode;
    std::vector<MTaskNode*> m_startTaskNode;  // 起始节点
    std::vector<MTaskNode*> m_finalTaskNode;  // 终止节点
    bool m_requireCompile;
    bool m_lock = false;  // 执行时锁定，防止修改
};
```

### 编译过程

`MTaskGraph::Compile()` 执行以下步骤：
1. 标记所有起始节点和终止节点
2. 反向 BFS 从终止节点遍历，计算每个节点的优先级
3. 优先级用于单线程执行时的顺序

### 执行器

#### 单线程遍历 (MSingleThreadTaskGraphWalker)
- 获取所有节点并按优先级排序
- 顺序执行每个节点
- 用于调试或线程禁用时

#### 多线程遍历 (MMultiThreadTaskGraphWalker)

```cpp
class MMultiThreadTaskGraphWalker
{
    enum class METaskState { Wait, Active, Finish };

    MThreadPool* m_threadPool;
    std::queue<MTaskNode*> m_waitTask;
    std::map<MTaskNode*, METaskState> m_nodeState;
    std::atomic_int m_activeTaskNum = 0;  // 原子操作计数活跃任务
    std::mutex m_taskStatehMutex;
};
```

**执行流程**:
1. 初始化所有起始节点为 Active 状态，加入待执行队列
2. 主循环:
   - 交换待执行队列与锁保护的队列
   - 对每个待执行节点，创建线程工作并提交给线程池
   - 节点执行完毕后调用 `OnTaskFinishedCallback`
   - 检查所有后继节点的依赖是否满足
   - 满足则添加到待执行队列
   - 当待执行队列和活跃任务数都为 0 时终止

---

## 4. 主线程与渲染线程分离

### 线程职责划分

```
┌─────────────────┐          ┌─────────────────┐
│   Main Thread   │          │  Render Thread  │
├─────────────────┤          ├─────────────────┤
│ - 游戏逻辑更新   │  Command  │ - GPU资源上传   │
│ - 资源加载管理   │◀────────▶│ - 实例缓冲更新  │
│ - 输入处理      │   Queue   │ - 渲染命令构建  │
│ - 组件通知      │          │ - 材质批处理    │
└─────────────────┘          └─────────────────┘
```

### 双缓冲命令队列模式

以网格实例管理器 (`MMeshInstanceManager`) 为例：

```cpp
struct UpdateCommand {
    enum class Type { AddGroup, RemoveGroup, AddInstance, RemoveInstance, UpdateInstance };
    Type type;
    MMeshInstanceKey proxyId;
    MMeshInstanceRenderProxy proxy;
    std::shared_ptr<MMaterialBatchGroup> batchGroup;
};

std::mutex m_updateMutex;
std::vector<UpdateCommand> m_updateQueue;      // 待处理队列
std::vector<UpdateCommand> m_pendingCommands;  // 待提交队列
```

**执行流程**:
1. **主线程 (SceneTick)**:
   - 加锁
   - 交换 `m_pendingCommands` 和 `m_updateQueue`
   - 解锁

2. **渲染线程 (RenderUpdate)**:
   - 加锁
   - 交换 `m_updateQueue` 到本地 commands
   - 解锁
   - 处理所有更新命令
   - 更新 GPU 存储缓冲

### 相关模块

| 模块 | 文件 | 同步机制 |
|------|------|----------|
| 网格实例管理器 | `Render/Batch/Mesh/MMeshInstanceManager.h/.cpp` | 命令队列 + 互斥锁 |
| 网格管理器 | `Render/Mesh/MMeshManager.h/.cpp` | 上传队列 + 互斥锁 |
| 渲染视图 | `Render/View/MRenderView.h` | 原子布尔标志 |

---

## 5. 异步资源加载系统

### 文件位置
- `Core/Resource/MResourceAsyncLoadSystem.h`
- `Core/Resource/MResourceAsyncLoadSystem.cpp`

### 架构设计

```cpp
class MResourceAsyncLoadSystem : public MISystem
{
    std::list<std::shared_ptr<MResourceLoader>> m_pendingLoader;   // 等待加载
    std::list<std::shared_ptr<MResourceLoader>> m_finishedLoader;  // 加载完成
    std::optional<MThreadWork> m_loadWork;  // 当前进行中的加载任务
};
```

### 两阶段加载流程

```
┌──────────────────────────────────────────────────────────┐
│                    资源加载流程                           │
├──────────────────────────────────────────────────────────┤
│  1. 主线程: 将资源加入 m_pendingLoader 队列               │
│  2. 工作线程: 执行 I/O 读取 (AnyThreadLoad)              │
│  3. 主线程: GPU资源创建和初始化 (MainThreadLoad)          │
└──────────────────────────────────────────────────────────┘
```

### 核心代码

```cpp
void MResourceAsyncLoadSystem::EngineTick(const float& delta)
{
    // 1. 处理已完成的资源加载
    if (!m_finishedLoader.empty())
    {
        MainThreadLoad(m_finishedLoader);  // 在主线程执行
        m_finishedLoader.clear();
    }

    // 2. 提交新的异步加载任务
    if (m_pendingLoader.empty()) return;

    constexpr size_t nResourceMaxLoadNum = 20;  // 每次最多加载20个

    std::list<std::shared_ptr<MResourceLoader>> vLoader;
    // 从待加载列表中取出最多20个

    m_loadWork = MThreadWork(METhreadType::EAny);
    m_loadWork.value().funcWorkFunction =
        M_CLASS_FUNCTION_BIND_1_0(
            MResourceAsyncLoadSystem::AnyThreadLoad,
            this,
            vLoader
        );
    pThreadPool->AddWork(m_loadWork.value());
}
```

### 特点

- 工作线程负责 I/O 密集的磁盘读取
- 主线程负责 GPU 资源创建
- 使用 `std::optional` 防止重复提交加载任务
- 批处理最多 20 个资源以平衡响应性

---

## 6. 引擎主循环集成

### 文件位置
- `Core/Engine/MEngine.h`
- `Core/Engine/MEngine.cpp`

### 主循环结构

```cpp
void MEngine::Tick(const float& delta)
{
    // 1. 系统更新 (主线程)
    for (auto& system: m_systemArray)
    {
        system->EngineTick(delta);
    }

    // 2. 任务图执行 (支持多线程)
    if (m_mainTaskGraph)
    {
        MMultiThreadTaskGraphWalker walker(&m_threadPool);
        m_mainTaskGraph->Run(&walker);
    }
}
```

### 执行层次

1. 所有系统的 `EngineTick()` 在主线程执行
   - 资源系统提交异步加载工作
   - 物理系统更新状态
   - 输入系统处理输入

2. 任务图并行执行
   - 主线程任务和渲染线程任务分离
   - 任务之间的依赖被正确处理

---

## 7. 同步机制汇总

### 同步原语使用

| 同步机制 | 用途 | 使用位置 |
|---------|------|----------|
| `std::mutex` | 保护共享数据 | MThreadPool, MMeshInstanceManager, MMeshManager |
| `std::condition_variable` | 线程唤醒通知 | MThreadPool |
| `std::unique_lock` | 锁定范围控制 | MThreadPool::ThreadRun |
| `std::lock_guard` | 自动 RAII 解锁 | MMultiThreadTaskGraphWalker, MMeshInstanceManager |
| `std::atomic_int` | 无锁计数 | MMultiThreadTaskGraphWalker::m_activeTaskNum |
| `std::atomic<bool>` | 无锁布尔标志 | MRenderView::m_submitFinished |
| `thread_local` | 线程本地存储 | MThreadPool::ThreadIndex |

### 锁策略

#### 最小锁时间
快速交换数据结构而非长期持有锁：
```cpp
std::lock_guard<std::mutex> lock(m_taskStatehMutex);
std::swap(vWaitTask, m_waitTask);
```

#### 双缓冲
- 主线程写入 `m_pendingCommands`
- 交换获得 `m_updateQueue`
- 渲染线程处理本地副本

#### 原子操作
计数器使用 atomic 避免锁开销

---

## 8. 当前限制

| 限制项 | 说明 |
|--------|------|
| 单线程模式 | `bSingleThreadMode = true`，默认同步执行 |
| 线程数限制 | 最多 10 个线程 (`M_MAX_THREAD_NUM`) |
| 无工作窃取 | 线程间无 Work Stealing 机制 |
| 无优先级队列 | FIFO 调度，无任务优先级 |
| 资源加载限制 | 每帧最多 20 个资源，无加载优先级 |

---

## 9. 关键代码文件

| 功能 | 文件 | 行数 |
|------|------|------|
| 线程池 | `Core/Thread/MThreadPool.h/.cpp` | ~130 |
| 工作结构 | `Core/Thread/MThreadWork.h` | ~40 |
| 任务图 | `Core/TaskGraph/MTaskGraph.h/.cpp` | ~300 |
| 任务节点 | `Core/TaskGraph/MTaskNode.h/.cpp` | ~200 |
| 多线程执行器 | `Core/TaskGraph/MMultiThreadTaskGraphWalker.h/.cpp` | ~130 |
| 单线程执行器 | `Core/TaskGraph/MSingleThreadTaskGraphWalker.h/.cpp` | ~50 |
| 异步资源加载 | `Core/Resource/MResourceAsyncLoadSystem.h/.cpp` | ~90 |
| 网格实例管理 | `Render/Batch/Mesh/MMeshInstanceManager.h/.cpp` | ~300 |
| 渲染视图 | `Render/View/MRenderView.h` | ~150 |

---

## 10. 架构优势

1. **模块化设计**: 线程池、任务图、资源系统相互独立
2. **灵活调度**: 可在单线程和多线程间切换
3. **任务依赖**: 自动处理任务间依赖关系
4. **两阶段加载**: I/O 和 GPU 初始化分离
5. **安全通信**: 使用互斥锁和原子操作
6. **渲染分离**: 明确的主线程/渲染线程边界
