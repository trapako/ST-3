#ifndef TIMEDDOOR_H
#define TIMEDDOOR_H

class TimerClient {
public:
    virtual void Timeout() = 0;
    virtual ~TimerClient() = default;
};

class Door {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool isDoorOpened() = 0;
    virtual ~Door() = default;
};

class TimedDoor;  // forward declaration

class DoorTimerAdapter : public TimerClient {
private:
    TimedDoor& door;
public:
    explicit DoorTimerAdapter(TimedDoor& d);
    void Timeout() override;
};

class TimedDoor : public Door {
private:
    DoorTimerAdapter* adapter;
    int iTimeout;
    bool isOpened;
public:
    explicit TimedDoor(int timeout);
    ~TimedDoor();
    bool isDoorOpened() override;
    void unlock() override;
    void lock() override;
    int getTimeOut() const;
    void throwState();  // выбрасывает исключение, если дверь открыта
};

class Timer {
private:
    TimerClient* client;
    void sleep(int seconds);  // имитация ожидания
public:
    Timer();
    void tregister(int timeout, TimerClient* cl);
};

#endif // TIMEDDOOR_H
