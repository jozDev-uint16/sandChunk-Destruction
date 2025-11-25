#pragma once

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "algorithm"

//  REFACTOR DRAFT OF THE CELL DATA STRUCTURE   V2
/*
    NOTE: "ptc" means particle
*/

#define MAX_BEHAVIORS 5

enum class ptcType : uint8_t{
    EMPTY,
    STONE,
    WOOD,
    GROUND,


    _MAX_TYPE_COUNT
};

typedef uint16_t (*behaviorAttribute)   (uint16_t ptc, ptcBox* box);


struct element{
    /* data */
    const char* name;
    ptcType     type;
    uint32_t    mass;

    uint8_t     bSteps;
    behaviorAttribute   behaviors[MAX_BEHAVIORS];

    element():  name(nullptr), type(ptcType::EMPTY), mass(0), bSteps(0) {  for (auto& b : behaviors) b = nullptr    };
    element(char* n, ptcType t, uint32_t grams, uint8_t steps, behaviorAttribute* give_behaviors):
        name(n),
        type(t),
        mass(grams),
        bSteps(steps)
    {   for (int i = 0; i < MAX_BEHAVIORS; i++) behaviors[i] = give_behaviors[i]};
};

//  INTENDED OUTCOME:   element(ptcType::SAND, 15 grams, 4 steps, {... assume sand behaviors}) -> represents what is "sand"

struct ptcComponent{
    /* data */
    bool    pAwake;
    element pElement;
    Color   pColor;
    Vector3 pShading;       //  tldr this is the HSV shading from the renderComp

    // does the ptc have to check its neighbors? probably not (handled by logic instead)
    ptcComponent(): pAwake(false), pElement(element()), pColor(BLANK) {};
    ptcComponent(element name, Color col, Vector3 hsv):
        pAwake      (true),
        pElement    (name),
        pColor      (col),
        pShading    (hsv)
    {};

};
struct kineticComponent{
    /* data */
    Vector2 velocity;
    float   kEnergy;
    element kElement;       // to access mass
};

//  INTENDED OUTCOME:   ptcComponent(Sand(...),DUNECOL,{.0f,.1f,.3f})   -> used for basic particle checks (phaseModel and reactionModel)
//  INTENDED OUTCOME:   kineticComponent()

class ptcWorld{
private:
    /* data */
public:
    ptcWorld(/* args */);
    ~ptcWorld();
};
class ptcBox{

    Vector2 xyBox;              // position of box in scene/world
    uint8_t bounds;             // in-grid limits of box -> max of 255
    uint16_t maxSize;           // max length of box list -> intended to be 255^2

    std::unique_ptr<ptcComponent[]> ptcCompos;
    std::unique_ptr<kineticComponent[]> kineticCompos;
    //  std::unique_ptr<renderComponent> renderComponent;

public:
    ptcBox(/* args */);
    ~ptcBox();
};


