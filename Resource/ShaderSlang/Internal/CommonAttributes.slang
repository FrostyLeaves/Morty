implementing InternalModule;

public enum PropertyType : int
{
    int,
    float,
    float3,
    float4,
    color
};

public enum InstanceType: int
{
    Property,
    TextureIndex
};

public enum ResourceType : int
{
    Texture2D,
    TextureCube
};

[__AttributeUsage(_AttributeTargets.Var)]
struct PropertyAttribute
{
    string displayName;
    PropertyType type;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct ResourceAttribute
{
    string displayName;
    ResourceType type;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct PerInstanceAttribute
{
    InstanceType type;
};

[__AttributeUsage(_AttributeTargets.Struct)]
struct StructLayoutAttribute
{
    string cppStructName;
};
