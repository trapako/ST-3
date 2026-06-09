// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TimedDoor.h"
#include <stdexcept>

using ::testing::_;
using ::testing::Mock;

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(5);
    }
    void TearDown() override {
        delete door;
    }
    TimedDoor* door;
};

TEST_F(TimedDoorTest, InitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 5);
}

TEST_F(TimedDoorTest, ThrowStateWhenOpenThrows) {
    door->unlock();
    EXPECT_THROW(door->throwState(), std::logic_error);
}

TEST_F(TimedDoorTest, ThrowStateWhenClosedDoesNotThrow) {
    door->lock();
    EXPECT_NO_THROW(door->throwState());
}

class DoorTimerAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(3);
        adapter = new DoorTimerAdapter(*door);
    }
    void TearDown() override {
        delete adapter;
        delete door;
    }
    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

TEST_F(DoorTimerAdapterTest, TimeoutWhenDoorOpenThrows) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::logic_error);
}

TEST_F(DoorTimerAdapterTest, TimeoutWhenDoorClosedDoesNothing) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST(TimerTest, TregisterStoresClient) {
    Timer timer;
    MockTimerClient mockClient;
    timer.tregister(1, &mockClient);
    SUCCEED();
}

class TestTimer : public Timer {
public:
    void tregister(int timeout, TimerClient* cl) override {
        cl->Timeout();
    }
};

TEST(TimerTest, TimerCallsTimeoutAfterRegister) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    
    TestTimer timer;
    timer.tregister(1, &mockClient);
}

class TimerSpy : public Timer {
public:
    void tregister(int timeout, TimerClient* cl) override {
        client = cl;
    }
    void fireTimeout() {
        if (client) client->Timeout();
    }
private:
    TimerClient* client = nullptr;
};

TEST(IntegrationTest, DoorOpenedThenTimerCausesException) {
    TimedDoor door(2);
    TimerSpy timer;
    DoorTimerAdapter adapter(door);
    
    door.unlock();
    timer.tregister(door.getTimeOut(), &adapter);
    
    EXPECT_THROW(timer.fireTimeout(), std::logic_error);
}

TEST(IntegrationTest, DoorClosedAfterUnlockNoException) {
    TimedDoor door(2);
    TimerSpy timer;
    DoorTimerAdapter adapter(door);
    
    door.unlock();
    door.lock();
    timer.tregister(door.getTimeOut(), &adapter);
    
    EXPECT_NO_THROW(timer.fireTimeout());
}

TEST(TimedDoorTest, DoubleUnlockKeepsOpen) {
    door->unlock();
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST(TimedDoorTest, DoubleLockKeepsClosed) {
    door->lock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}
