#pragma once

#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "memory"
#include "vector"
#include "algorithm"

#define  PTC_SCALE      3
#define  AIR_DRAG_BASE  0.98f
#define  GRAVITY        0.15f

enum class elementID : uint8_t{
    AIR,
    SAND,

    //
    _MAX_ID
};  
enum checkBitmap : uint8_t{
    DOWN    = 1 << 0,
    LEFT    = 1 << 1,
    RIGHT   = 1 << 2,
    UP      = 1 << 3,
    DOWNLEFT    = 1 << 4,
    DOWNRIGHT   = 1 << 5,
    UPLEFT      = 1 << 6,
    UPRIGHT     = 1 << 7
};
constexpr int collideNeighbors [8][2] = {
    {0,1},{-1,0},{1,0},{0,-1},      // DOWN, LEFT, RIGHT, UP (cardinals)
    {-1,1},{1,1},{-1,-1},{1,-1}     // DL, DR, UL, UR        (diagonals)
};
enum flagBitmap : uint8_t{
    ASLEEP      = 0,
    AWAKE       = 1 << 0,
    IS_KINETIC  = 1 << 1,
    IS_STAINED  = 1 << 2
    //
};

struct phaseCompo{
    /* data */
    enum class phaseType{
        STATIC, POWDER, FLUID, VAPOR
    }       phase;
    float   mass;           // momentum gain, displacement (liquid), inertia when collided 
    float   friction;       // momentum loss on slopes,
};
struct elementCompo{
    /* data */
    const char* display;    // string name display for debug/brush
    Color       color;
    phaseCompo  phaseAttributes;
};
struct kineticCompo{
    /* data */
    Vector2     velocity;
    float       inertia;   //  tendency to stay static (collision) or keep moving (momentum)

    void resetPhysics(){
        this->inertia   = 0.0f;
        this->velocity  = {0.0f,0.0f};
    }
};

struct particle{
    /* static data */
    elementID       type;

    /* dynamic data */
    uint8_t         neighbors;
    uint8_t         state;
    kineticCompo    kinematics;

    void setState   (bool activate, flagBitmap selected){
        if (activate)   this->state |= selected;
        else            this->state &= ~selected;
    }     
    bool checkState (flagBitmap target) const {
        return          ((this->state & target) != 0);
    }             

    // CONSTRUCTOR: for empty particles
    particle(){ 
        this->type = elementID::AIR;
        this->neighbors = 0U;
        this->kinematics = { };
        setState(true, flagBitmap::ASLEEP);
    }
    // CONSTRUCTOR: for non-empty particles
    particle(elementID element){ 
        this->type = element;
        this->neighbors = 0U;
        this->kinematics = { };
        setState(true, flagBitmap::AWAKE);
    }
};

extern std::vector<elementCompo> elementRegistry;
void initialElements();

class  particleBox{
private:
    /* data */   
    uint16_t    maxPtcs;    
    uint8_t     boundsPtcs; 

    Image       ptcTxtr;
    Texture2D   ptcTxtrDrawn;

public:
    Vector2     boxScreenPos;
    uint8_t     scalePtcs;
    std::unique_ptr<particle[]> particles;
    
    particleBox(Vector2 screenxy, uint8_t scale){
        this->boxScreenPos  = screenxy,
        this->scalePtcs     = scale,
        this->maxPtcs       = UINT16_MAX,
        this->boundsPtcs    = UINT8_MAX;

        this->particles     = std::make_unique<particle[]>(this->maxPtcs);
        for (int i = 0; i < this->maxPtcs; i++) this->particles[i] = particle();

        this->ptcTxtr       = GenImageColor (boundsPtcs,boundsPtcs,BLANK);
        this->ptcTxtrDrawn  = LoadTextureFromImage (this->ptcTxtr);
    };
    ~particleBox(){
        UnloadImage     (this->ptcTxtr);
        UnloadTexture   (this->ptcTxtrDrawn);
    };

    // SOLVERS
    void    ballisticSolver  (int idx, const elementCompo define);
    void    powderSolver     (int idx, const elementCompo define); 

    // CORE METHODS
    int     getPtcOffset    (int idx, checkBitmap target, uint8_t range);  
    void    particleUpdate  (int idx);
    void    particleSwap    (int idxA, int idxB);
    void    particleAdd     (int x, int y, elementID type);
    void    particleWake    (int idx);

    // BOX METHODS
    void    boxUpdate       ();
    void    boxRender       (bool debug, int zoom);
    void    boxReset        (bool activate);
    void    boxDebugs       ();
    
};



