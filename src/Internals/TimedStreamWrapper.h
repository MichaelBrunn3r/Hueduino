#pragma once

#include <Arduino.h>
#include <Stream.h>

#define CONCATE_(X, Y) X##Y
#define CONCATE(X, Y) CONCATE_(X, Y)

#define ALLOW_ACCESS(CLASS, TYPE, MEMBER) \
  template<typename Only, TYPE CLASS::*Member> \
  struct CONCATE(MEMBER, __LINE__) { friend TYPE (CLASS::*Access(Only*)) { return Member; } }; \
  template<typename> struct Only_##MEMBER; \
  template<> struct Only_##MEMBER<CLASS> { friend TYPE (CLASS::*Access(Only_##MEMBER<CLASS>*)); }; \
  template struct CONCATE(MEMBER, __LINE__)<Only_##MEMBER<CLASS>, &CLASS::MEMBER>

#define ACCESS(OBJECT, MEMBER) (OBJECT).*Access((Only_##MEMBER<std::remove_reference<decltype(OBJECT)>::type>*)nullptr)

ALLOW_ACCESS(Stream, unsigned long, _timeout);

namespace Hueduino {
    namespace Internals {
        class TimedStreamWrapper : public Stream {
            public:
                TimedStreamWrapper(Stream& stream) : mStream(stream) {
                    _timeout = ACCESS(stream, _timeout);
                }

                virtual int available() { 
                    return mStream.available();
                }

                virtual int read() {
                    int c;
                    _startMillis = millis();
                    do {
                        c = mStream.read();
                        if(c >= 0) return c;
                        yield();
                    } while(millis() - _startMillis < _timeout);
                    return -1; // -1 indicates timeout
                }

                virtual int peek() {
                    int c;
                    _startMillis = millis();
                    do {
                        c = mStream.peek();
                        if(c >= 0) return c;
                        yield();
                    } while(millis() - _startMillis < _timeout);
                    return -1; // -1 indicates timeout
                }

                virtual size_t write(uint8_t byte) {
                    return mStream.write(byte);
                }

                unsigned int getTimeout() {
                    return _timeout;
                }
            private:
                Stream& mStream;
        };
    } // Internals
} // Hueduino

#undef ALLOW_ACCESS
#undef ACCESS
#undef CONCATE_
#undef CONCATE