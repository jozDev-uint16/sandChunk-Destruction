#pragma once

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "vector"
#include "algorithm"

#define GRAVITY 0.25f
//  REFACTOR DRAFT OF THE CELL DATA STRUCTURE   V2
/*
    NOTE: "ptc" means particle
*/

enum class ptcType : uint8_t{
    EMPTY,
    STONE,
    WOOD,
    GROUND,
    SAND,


    _MAX_TYPE_COUNT
};

enum ptcFlags : uint8_t{
    FLAG_SLEEP      = 0,
    /*  the rest must be bitshifts! */
    FLAG_AWAKE      = 1 << 0,
    FLAG_BALLISTIC  = 1 << 1,
    FLAG_IGNITED    = 1 << 2

};


class ptcBox;
typedef bool (*rxnFunction)(int actorIdx, int targetIdx, ptcBox* rxnbox);

//  STATIC COMPOS: element and phase
//  DYNAMIC COMPOS: kinetic (changes in runtime)

struct phaseComponent{
    /* data */
    enum class phaseType {  SOLID, POWDER, LIQUID, VAPOR } phase;
    float density, adhesion, elasticity;    
    // refers to displacement (heavy > light), friction, and bounciness

};
struct elementComponent{
    /* data */
    const char*     name;
    Color           color;          // FLAGGED: Will be replaced with textures
    phaseComponent  phaseAttribs;

    // listed as a global registry: registry[typeID]
};
struct kineticComponent{
    /* data */
    Vector2 velocity;
    float   inEnergy;

    /* helpers */
    void resetK(){  
        velocity = {0,0}; 
        inEnergy = 0;  
    };
    //  resets motion to zero
    void addDrag(float amt){
        if(inEnergy > 0){ 
            inEnergy -= amt;
            if(inEnergy < 0) inEnergy = 0;
        };
    };
};

struct particle{
    /* NOTE: "ptc" means particle */
    uint8_t typeID, flags;
    kineticComponent kinems;

    /*  helpers */
    bool isKinetic()     { return (flags & FLAG_BALLISTIC) != 0; };

    void setKinetic(bool val) {
        if (val) flags |= FLAG_BALLISTIC;
        else     flags &= ~FLAG_BALLISTIC;
    }
    void setAwake(bool val) {
        if (val) flags |= FLAG_AWAKE;
        else     flags &= ~FLAG_AWAKE;
    }
    
    particle();
    particle(ptcType element);
};


//  INTENDED OUTCOME:   kineticComponent()                              -> used for kinematic motions (gravity and friction)

extern std::vector<rxnFunction>         reactionRegistry;
void registerRxns(ptcType actor, ptcType target, rxnFunction fx);
void initialRxnMatrix();

extern std::vector<elementComponent>    elementRegistry;
void initialElements();

class ptcBox{

    uint8_t bounds;             // in-grid limits of box -> max of 255

public:
    Vector2 xyBox;              // position of box in scene/world
    uint16_t maxSize;           // max length of box list -> intended to be 255^2
    uint8_t ptcScale;           // how large?

    particle* particles;        // raw pointer array (since destructor will just dealloc)

    ptcBox(Vector2 xy, uint8_t scale);
    ~ptcBox();

    /*  CORE METHODS*/
    void    updatePtc   (int idx);
    void    swapPtc     (int idxA, int idxB);
    int     getPtcOffset(int idx, int dx, int dy);

    /*  SOLVERS -> solves behavior outcomes */
    void    kineticSolver   (int idx);
    void    rollingSolver   (int idx, int dir);
    void    regularSolver   (int idx);

    /*  HANDLERS -> determines effects */
    void    handleImpact    (int kinetic, int target);

    /*  BOX: self explanatory   */
    void    boxUpdate   (int fCount);
    void    boxDraw     (bool debugMode);
    void    boxAdd      (int x, int y, ptcType element);
    void    boxClear    ();

    void    boxDebug    (bool debugActivate);
};


