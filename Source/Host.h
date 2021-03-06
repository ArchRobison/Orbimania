/* Copyright 1996-2016 Arch D. Robison 

   Licensed under the Apache License, Version 2.0 (the "License"); 
   you may not use this file except in compliance with the License. 
   You may obtain a copy of the License at 
   
       http://www.apache.org/licenses/LICENSE-2.0 
       
   Unless required by applicable law or agreed to in writing, software 
   distributed under the License is distributed on an "AS IS" BASIS, 
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
   See the License for the specific language governing permissions and 
   limitations under the License.
 */

/******************************************************************************
 OS specific services on the host that are called from OS-independent game code.
*******************************************************************************/

#include <string>

// Types defined in NimbleDraw.h
class NimbleColor;
class NimblePixMap;
class NimblePoint;
class NimbleRect;

//! Return current absolute time in seconds.
/** Only the difference between two calls are meaningful, because the 
    definition of 0 is platform dependent. */
double HostClockTime();

/** 0 = no limit
    1 = one per frame
    2 = every two frames */
void HostSetFrameIntervalRate( int limit );

//! Estimate on how busy host is maintaining current frame rate with current resources.
/** 0.75 means that host is idling 25% of the time. */
float HostBusyFrac();

//! Enumeration of keys corresponding to non-printing characters. 
enum {
    HOST_KEY_RETURN=0xD,
    HOST_KEY_ESCAPE=0x1B,
    HOST_KEY_LEFT = 256,
    HOST_KEY_RIGHT,
    HOST_KEY_UP, 
    HOST_KEY_DOWN,
    HOST_KEY_DELETE,
    HOST_KEY_LAST           // Value for declaring arrays
};

//! Return true if specified key is down, false otherwise.
/** Use one of the HOST_KEY_* enumeration for keys that do not correspond to printable ASCII characters.
    The value of key should be lowercase if it is alphabetic. */
bool HostIsKeyDown( int key );

//! Called by client game to either show (show=true) or hide (show=false) the cursor
/** Not used by Seismic Duck, but retained because Frequon Invaders needs it. */
void HostShowCursor( bool show );

//! Request termination of the game.
void HostExit();

class BuiltFromResourcePixMap;

//! Load a resource.
/** Construct map for resource with name item.resourceName().
    Call item.buildFrom(map) */
void HostLoadResource( BuiltFromResourcePixMap& item );

// Note: text argumentsa are assummed to be UTF8
class HostFont {
    HostFont(const HostFont&) = delete;
    void operator=(const HostFont&) = delete;
    // void* to hide underlying OS service
    void* myFont;
public:
    HostFont() : myFont(nullptr) {}
    void open(const char* filename, int ptsize);
    void close();
    bool isOpen() const {return myFont!=nullptr;}
    // Return nominal height of font
    int height() const;
    // Return width and height of a bounding box for the text.
    NimblePoint size( const char* text ) const;
    // Draw text and return bounding box for the text.
    NimbleRect draw(NimblePixMap& map, int x, int y, const char* text, const NimbleColor& color) const;
};

enum class HostGetFileNameOp {
    create,
    open,
    saveAs
};

//! Ask user for a filename.  Returns empty string on failure.
std::string HostGetFileName(HostGetFileNameOp op, const char* fileType, const char* fileSuffix);
