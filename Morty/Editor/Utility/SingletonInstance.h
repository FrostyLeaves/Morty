#pragma  once

template<typename CLASS>
class SingletonInstance
{
public:

    static CLASS* GetInstance()
    {
        static CLASS instance;
        return &instance;
    }
};