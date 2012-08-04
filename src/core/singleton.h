#ifndef SINGLETON_H
#define SINGLETON_H

template<class T>
class Singleton
{
public:
    static T* instance()
    {
        if ( !instance_ )
            instance_ = new T();

        return instance_;
    }

    static void release()
    {
        if ( !instance_ )
            delete instance_;
        instance_ = NULL;
    }
    

private:
    Singleton();
    Singleton(const Singleton&);
    T& operator=(const Singleton&);

    virtual ~Singleton();


private:
    static T* instance_; 
};

template<class T>
T* Singleton<T>::instance_ = NULL; 

#endif //SINGLETON_H
