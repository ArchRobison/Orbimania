#ifndef Handle_H
#define Handle_H

/*
A Handle is something the cursor can grab.  
*/
struct Handle {
    enum kindType: char {
        null,
        tailFull,       // Tail of arrow
        head,           // Head of arrow
        circle,         // circle for mass
        tailHollow      // Hollow tail of arrow
    };
    static const unsigned maskAll = ~0u;
    static const unsigned maskTail = 1<<tailFull | 1<<tailHollow;

    bool isNull() const {return kind==null;}
    // Kind of handle
    kindType kind;
    // Index into Universe::StateVar
    size_t index;
    Handle() : kind(null), index(-1) {}
    Handle( Handle::kindType kind_, size_t index_ ) : index(index_), kind(kind_) {}
    bool match( Handle::kindType kind_, size_t index_ ) const {return kind==kind_ && index==index_;}
};

void HandleBufClear();
void HandleBufAdd( float x, float y, float r, const Handle& h );
void SelectHandle(int x, int y);
void UpdateHandlesAfterErasure(size_t k);
extern Handle SelectedHandle;

#endif /* Handle_H */
