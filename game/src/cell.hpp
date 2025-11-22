#pragma once

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "algorithm"
#include "iostream"
#include "vector"

#define CELL_SCALE 3
#define MAX_ELEM_STEPS 4

enum cName : uint8_t{
    NONE = 0,
    SAND,
    GRAVEL,
    WOOD,
    /*  NOTE: do not use the last enum as an actual particle    */
    MAX_TYPES
};

#define SANDCOL     (Color){237, 211, 157, 255}
#define GRAVELCOL   (Color){95, 94, 93, 255}
#define WOODCOL     (Color){95, 62, 42, 255}

struct cellOffset { int dx, dy; };

static constexpr cellOffset NEIGHBORS[8] = {
    {0, 1},   // DOWN       = 1 << 0,  // 0b00000001 (0x01)
    {-1, 1},  // DOWN_LEFT  = 1 << 1,  // 0b00000010 (0x02)
    {1, 1},   // DOWN_RIGHT = 1 << 2,  // 0b00000100 (0x04)
    {-1, 0},  // LEFT       = 1 << 3,  // 0b00001000 (0x08)
    {1, 0},   // RIGHT      = 1 << 4,  // 0b00010000 (0x10)
    {0, -1},  // UP         = 1 << 5,  // 0b00100000 (0x20)
    {-1, -1}, // UP_LEFT    = 1 << 6,  // 0b01000000 (0x40)
    {1, -1}   // UP_RIGHT   = 1 << 7,  // 0b10000000 (0x80)

    /*
    // Optional: Define useful combinations
    FALLING    = DOWN | DOWN_LEFT | DOWN_RIGHT, // 0x07
    HORIZONTAL = LEFT | RIGHT,                  // 0x18
    ALL        = 0xFF
    */
};
static const uint8_t ORDER_NORMAL[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const uint8_t ORDER_SWAPPED[8] = { 0, 2, 1, 3, 4, 5, 6, 7 };
/*      */

struct cellDomain;      // forward declared

typedef std::size_t (*behaviorLogic)(std::size_t cell, cellDomain* domain); 
typedef Color (*renderAfter)(std::size_t cell, cellDomain* domain); 

struct renderComp{
    Color color;
    Vector3 dHSV;
    // TODO: add a texture mask somehow...

    renderAfter post;        // function pointer to a generic method for graphic (unique to specific types)

    renderComp() : renderComp({0, 0, 0, 0}, {0.0f, 0.0f, 0.0f}, nullptr) {};   // default constructor -> shows nothing!

    renderComp(Color c,  Vector3 dc, renderAfter lastcol): 
        color(c), 
        dHSV(dc), 
        post(lastcol) 
    {};
};

struct behaviorComp{
    
    bool active;            // if active, then update! (and the converse)
    cName type;
    uint8_t collideMask;    // bitmap of needed neighboring cells
    float density;   
    // add more metrics maybe?       

    behaviorComp(): active(false), type(NONE), collideMask(0b00000000), density(0.0f) {};

    behaviorComp(cName t, uint8_t mask): 
        active(true),          // ⬅️ Initialized (e.g., set to true for a new particle)
        type(t),               
        collideMask(mask),     
        density(1.0f)         // ⬅️ Initialized (e.g., use a default value, or take as argument)   
    {};
};

struct elementRecipe{
    uint8_t count;
    behaviorLogic bSteps[MAX_ELEM_STEPS];
};



/*  CELL DOMAIN   */

struct cellDomain{
    /* BOUNDARIES */
    std::size_t xBounds, yBounds, maxBounds;
    /* METADATA */
    uint16_t cellSize;
    Vector2 boxPos;

    // NEW: The index IS the cell

    std::unique_ptr<renderComp[]> renderCompCells;
    std::unique_ptr<behaviorComp[]> behaviorCompCells;

    /*  CONSTRUCTOR  */

    cellDomain(std::size_t xb, std::size_t yb, uint16_t c, Vector2 p): 
        xBounds(xb), yBounds(yb), maxBounds(xb * yb), cellSize(c), boxPos(p), 
        renderCompCells(new renderComp[maxBounds]), behaviorCompCells(new behaviorComp[maxBounds]) {};

    std::size_t readIndex   (std::size_t x, std::size_t y) const; 
    bool checkBounds        (std::size_t x, std::size_t y) const; 
        
    renderComp& readRender      (std::size_t i); 
    behaviorComp& readBehavior  (std::size_t i) ;

    /*  METHODS   */

    std::size_t getOffset(std::size_t cell, std::size_t dx, std::size_t dy);

    void cellSwap   (std::size_t cellA, std::size_t cellB);
    void cellAdd    (std::size_t cx, std::size_t cy, cName id);
    void cellUpdate (std::size_t x, std::size_t y);

    void domainUpdate (int frameCount);
    void domainRender ();
    void domainReset  ();

};

//  INITIALIZE NEW PARTICLES HERE

extern elementRecipe updateManager[MAX_TYPES];

extern behaviorComp updateList[MAX_TYPES];
extern renderComp renderList[MAX_TYPES];

void initElementRecipe();
void initCellPalette();

/*  This basically makes the order of all added things indexed by the enum + 1*/

//  INITIALIZE THE CHUNK HERE TOO:

