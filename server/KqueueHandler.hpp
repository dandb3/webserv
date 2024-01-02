#ifndef KQUEUEHANDLER_HPP
#define KQUEUEHANDLER_HPP

/*
**  KqueueHandler
**  Kqueue에 대한 이벤트를 처리하는 클래스
**
*/
class KqueueHandler
{
private:
public:
    KqueueHandler();
    KqueueHandler(const KqueueHandler &ref);
    ~KqueueHandler();

    KqueueHandler &operator=(const KqueueHandler &ref);
};

#endif
