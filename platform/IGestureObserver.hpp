#ifndef IGESTUREOBSERVER_HPP__
#define IGESTUREOBSERVER_HPP__


struct IGestureObserver
{
    virtual void tap(float x, float y) = 0;
    virtual void pan(float x, float y, float dx, float dy) = 0;
    virtual void pinch(float scale, bool is_end) = 0;
protected:
    virtual ~IGestureObserver() {}
};


#endif // IGESTUREOBSERVER_HPP__