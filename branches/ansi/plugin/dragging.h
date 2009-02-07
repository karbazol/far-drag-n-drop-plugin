/**
 * @file dragging.h
 * Contains declaration of Dragging class
 *
 * $Id: dragging.h 56 2008-05-11 15:16:07Z eleskine $
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__DNDPRPR_H__
#define __KARBAZOL_DRAGNDROP_2_0__DNDPRPR_H__

/**
 * @brief Dragging state singleton
 *
 * Singleton. Represents the state of dragging operation.
 */
class Dragging
{
private:
    bool _initialized;
    bool _dragging;
private:
    Dragging():_initialized(false),_dragging(false){}
    Dragging(const Dragging&);
    ~Dragging(){}
    static void kill(Dragging* p);
public:
    static Dragging* instance();
    void initialize();
    void shutDown();
    inline bool isReadyForDragging(){return _initialized && !_dragging;}
    bool start();
    inline void draggingStarted(){_dragging = true;}
    inline void draggingEnded(){_dragging = false;}
    inline bool dragging(){return _dragging;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DNDPRPR_H__
// vim: set et ts=4 ai :

