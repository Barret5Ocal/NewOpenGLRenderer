void B50memcpy(void * destination, const void * source, size_t num )
{
    char *D = (char *) destination;
    char *S  = (char *) source; 
    for(int Index = 0;
        Index < num;
        ++Index)
    {
        D[Index] = S[Index];
    }
}

template <typename T> 
struct b50_list
{
    int Num;
    T *Ptr;
    
    void Append(T Thing)
    {
        int Size = sizeof(T);
        if(Ptr)
        {
            T *Temp = (T *)malloc(Size * Num + 1);
            
            memcpy(Temp, Ptr, Size * Num);
            
            free(Ptr);
            Ptr = Temp;
            
            *(Ptr + (Num * Size)) = Thing;
            
            Num++;
        }
        else
        {
            Ptr = (T *)malloc(Size);
            *Ptr = Thing;
            Num = 1;
        }
    }
    
    T operator[](int x)
    {
        return Ptr[x];
    }
};

struct memory_arena
{
    unsigned char *Memory; 
    size_t Size;
    size_t Used;
};

void InitMemoryArena(memory_arena *Memory, size_t Size)
{
    
#if defined(_WIN32)
    Memory->Memory = (unsigned char *)VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Memory->Size = Size;
    Memory->Used = 0;
#elif defined(__APPLE__)
    
    Memory->Memory = (unsigned char *)malloc(Size);
    Memory->Size = Size;
    Memory->Used = 0;
#else
    
    Memory->Memory = (unsigned char *)malloc(Size);
    Memory->Size = Size;
    Memory->Used = 0;
    
#endif
}

#define PushArray(Arena, Count, Type) PushSize(Arena, sizeof(Type) * Count)
#define PushStruct(Arena, Type) PushSize(Arena, sizeof(Type))

void *PushSize(memory_arena *Arena, size_t Size)
{
    void *Result = 0;
    if((Size + Arena->Used) < Arena->Size)
    {
        Result = Arena->Memory + Arena->Used; 
        Arena->Used += Size;
    }
    else 
        Assert(false);
    return Result; 
}

memory_arena PushArena(memory_arena *Arena, size_t Size)
{
    memory_arena Result = {};
    if((Size + Arena->Used) < Arena->Size)
    {
        Result.Memory = Arena->Memory + Arena->Used; 
        Result.Used = 0;
        Arena->Used += Size;
        Result.Size = Size; 
    }
    else 
    {
        Assert(false);
    }
    return Result; 
    
}

void ResetArena(memory_arena *Arena)
{
    Arena->Used = 0;
}