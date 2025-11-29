#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "vector"
#include "algorithm"
#include "iostream"


#include "particle.hpp"


std::vector<elementComponent> elementRegistry;
void initialElements(){
    elementRegistry.resize((int)ptcType::_MAX_TYPE_COUNT);

    // Phase, Density, Friction, Restitution  
    
    elementRegistry[(int)ptcType::STONE] = { 
        "Stone", (Color){89, 89, 90, 255}, 
        { phaseComponent::phaseType::SOLID, 3.5f, 0.5f, 0.1f } 
    };
    elementRegistry[(int)ptcType::WOOD] = { 
        "Wood", (Color){95, 62, 42, 255}, 
        { phaseComponent::phaseType::SOLID, 1.5f, 0.3f, 0.2f } 
    };
    elementRegistry[(int)ptcType::GROUND] = { 
        "Ground", (Color){80, 50, 30, 255}, 
        { phaseComponent::phaseType::SOLID, 3.0f, 0.5f, 0.0f } 
    };
    elementRegistry[(int)ptcType::SAND] = { 
        "Sand", (Color){237, 211, 157, 255}, 
        { phaseComponent::phaseType::POWDER, 2.0f, 0.5f, 0.2f } 
    };
}

//

std::vector<rxnFunction> reactionRegistry;

bool NoReaction(int actorIdx, int targetIdx, ptcBox* rxnbox) {    return false;  }
// help(er)
rxnFunction getRxn(ptcType actor, ptcType target) {
    return reactionRegistry[(int)actor * (int)ptcType::_MAX_TYPE_COUNT + (int)target];
}
void        registerRxns(ptcType actor, ptcType target, rxnFunction fx){
    reactionRegistry[(int)actor * (int)ptcType::_MAX_TYPE_COUNT + (int)target] = fx;
}
void        initialRxnMatrix(){
    int maxRxns = (int)(ptcType::_MAX_TYPE_COUNT);
    reactionRegistry.resize( maxRxns * maxRxns, NoReaction);
    /*  thats it for now, blank reacts    */
};



//

particle::particle(){
    this->typeID = (uint8_t)ptcType::EMPTY;
    this->kinems.resetK();

    this->setAwake(false);
};

particle::particle(ptcType element){
    this->typeID = (uint8_t)element;
    this->kinems.resetK();

    this->setAwake(true);
}

ptcBox::ptcBox(Vector2 xy, uint8_t scale){
    this->xyBox = xy, 
    this->bounds = 255, 
    this->maxSize = 255*255, 
    this->ptcScale = scale, 
    this->particles = new particle[maxSize];

    for (int i = 0; i < maxSize; i++){ this->particles[i] = particle();};
};

ptcBox::~ptcBox(){  
    delete[] particles;
    particles = nullptr;
};

/*  CORE METHODS*/
void    ptcBox::updatePtc   (int idx){
    particle& ptc = this->particles[idx];

    if (!(ptc.flags & FLAG_AWAKE)) return;

    if (ptc.isKinetic()){
        this->kineticSolver(idx);   // kinetic ptc (>1 px movement)
    }
    else{
        this->regularSolver(idx);
    };
};
void    ptcBox::swapPtc     (int idxA, int idxB){
    bool ValidA = (idxA < this->maxSize);
    bool ValidB = (idxB < this->maxSize);

    if (!ValidA || !ValidB) return;

    std::swap(this->particles[idxA],this->particles[idxB]);
};
int     ptcBox::getPtcOffset(int idx, int dx, int dy){
    int current_x = (idx % bounds);
    int current_y = (idx / bounds);

    int newx = current_x + dx;
    int newy = current_y + dy;

    if (newx < 0 || newx >= bounds || 
        newy < 0 || newy >= bounds) return -1;
    //  INVALID if out of bounds

    return ((newy * bounds) + newx);
};

/*  SOLVERS -> solves behavior outcomes */
void    ptcBox::kineticSolver   (int idx){
    particle& ptc = this->particles[idx];

    //  STEP 1: Delta Velocity 
    ptc.kinems.velocity.x *= 0.99f;
    ptc.kinems.velocity.y += GRAVITY;

    float speeeeed = sqrtf((ptc.kinems.velocity.x * ptc.kinems.velocity.x)+(ptc.kinems.velocity.y * ptc.kinems.velocity.y)); // long ass equation

    //  STEP 2: Sub Pixel Speed
    if (speeeeed < 1.0f)    {
        /* code */
        ptc.setKinetic(false);
        ptc.kinems.inEnergy = speeeeed;
        return;
    }   // sub 1 pixel movement turns particles back into regular (smart) ones

    //  STEP 3: Raycast Vector
    int steps = (int)ceilf(speeeeed);
    float stepX = ptc.kinems.velocity.x / steps;
    float stepY = ptc.kinems.velocity.y / steps;

    int nowIdx = idx;

    for (int v = 0; v < steps; v++) {
        /* code */
        int nextIdx = this->getPtcOffset(nowIdx, (int)roundf(stepX), (int)roundf(stepY));

        //  3.1:    hits box border
        if (nextIdx == -1) {
            /*  FLAGGED: will be revamped when chunks are introduced!   */
            ptc.setKinetic(false);
            ptc.kinems.resetK();
            return;
        }

        //  3.2:    hits another particle
        if (this->particles[nextIdx].typeID != (uint8_t)ptcType::EMPTY) {
            handleImpact(nowIdx,nextIdx);
            return;
        }

        //  3.3:    update position
        this->swapPtc(nowIdx, nextIdx);
        nowIdx = nextIdx;
    }
};
void    ptcBox::rollingSolver   (int idx, int dir){
    particle& ptc = this->particles[idx];

    int maxSteps = (int)ptc.kinems.inEnergy;
    if (maxSteps > 5) maxSteps = 5; // Cap rolling speed per frame

    int nowIdx = idx;

    for (int i = 0; i < maxSteps; i++) {
        
        // A. Ramp Launch Check (Ground disappeared)
        int down = this->getPtcOffset(nowIdx, 0, 1);
        if (down != -1 && this->particles[down].typeID == (uint8_t)ptcType::EMPTY) {
            // Convert Roll -> Ballistic
            ptc.kinems.velocity.x = (float)(dir * ptc.kinems.inEnergy);
            ptc.kinems.velocity.y = 0.5f;
            ptc.setKinetic(true);
            break;
        }

        // B. Pathfinding (Diagonal Priority)
        int diag = this->getPtcOffset(nowIdx, dir, 1);
        
        // OBSTACLE HIT CHECK
        if (diag != -1 && this->particles[diag].typeID != (uint8_t)ptcType::EMPTY) {
            // Ski Ramp Logic: Can we fly over it?
            if (ptc.kinems.inEnergy > 4.0f) {
                // Yes! Launch horizontally
                ptc.kinems.velocity.x = (float)dir * ptc.kinems.inEnergy;
                ptc.kinems.velocity.y = -1.0f; // Hop up
                ptc.setKinetic(true);
                
                // Optional: Kick the obstacle (Dislodge)
                this->handleImpact(nowIdx, diag);
                break;
            }
            break; // Blocked and not enough energy
        }
        else if (diag != -1) {
            // Path clear, Roll
            this->swapPtc(nowIdx, diag);
            nowIdx = diag;
            ptc.kinems.inEnergy -= 0.1f; // Rolling cost
        }
        else {
            // Check flat side (Liquids mainly)
            int side = this->getPtcOffset(nowIdx, dir, 0);
            if (side != -1 && this->particles[side].typeID == (uint8_t)ptcType::EMPTY) {
                 swapPtc(nowIdx, side);
                 nowIdx = side;
                 ptc.kinems.inEnergy -= 0.2f;
            } else {
                break; // Stuck
            }
        }
    }
};
void    ptcBox::regularSolver   (int idx){
    particle& ptc = this->particles[idx];
    const elementComponent& def = elementRegistry[(int)ptc.typeID];

    if (def.phaseAttribs.phase == phaseComponent::phaseType::SOLID){ 
        ptc.kinems.inEnergy = 0;
        return;
    }    // SOLIDS dont move (insta skip)

    //  Simple Gravity (1px)
    int downer = this->getPtcOffset(idx,0,1);
    bool fell = false;

    if (downer != -1) {
        if (particles[downer].typeID == (uint8_t)ptcType::EMPTY) {
            swapPtc(idx, downer);
            ptc.kinems.inEnergy += GRAVITY;


            bool fell = true;
            if (ptc.kinems.inEnergy < 1.0f) return;

            ptc.setKinetic(true); 
                
            // CRITICAL: Transfer the scalar Energy into Vector Velocity
            ptc.kinems.velocity.y = ptc.kinems.inEnergy;

            return;
        }
    }

    if (!fell) {
        // 1. Apply heavy damping (e.g. lose 50% energy immediately upon hitting ground)
        ptc.kinems.inEnergy *= 0.5f;
        
        // 2. Apply static friction threshold (The "Adhesion" property)
        ptc.kinems.inEnergy -= def.phaseAttribs.adhesion;

        // 3. Clamp to 0 (Prevent negative energy)
        if (ptc.kinems.inEnergy < 0.0f) {
            ptc.kinems.inEnergy = 0.0f;
        }
    };
    // 2. Rolling Logic (Surface Crawl)
    ptc.kinems.addDrag(def.phaseAttribs.adhesion);

    if (ptc.kinems.inEnergy <= 0) {
        ptc.setAwake(false);
        return;
    };
    // Determine Direction based on X velocity or Random
    int dir = (ptc.kinems.velocity.x != 0) ? ((ptc.kinems.velocity.x > 0) ? 1 : -1) : ((GetRandomValue(0,1)==0)?1:-1);
    rollingSolver(idx, dir);
};


/*  HANDLERS -> determines effects */
void    ptcBox::handleImpact    (int kinetic, int target){
    particle& kPtc = particles[kinetic];    // the ballistic
    particle& tPtc = particles[target];     // the victim*

    const elementComponent& targetElement = elementRegistry[tPtc.typeID];
    const elementComponent& kineticElement = elementRegistry[kPtc.typeID];

    kPtc.setKinetic(false);

    // 2: Check SOLID things
    if (targetElement.phaseAttribs.phase == phaseComponent::phaseType::SOLID)    {
        /* FLAGGED: Some static elements will eventually be implemented to convert it into
            dislodged powders (ie stone -> gravel) */
        kPtc.kinems.resetK();
        return;
    }
    
    // 3: Energy Transfer
    float transferRatio = (kineticElement.phaseAttribs.density / targetElement.phaseAttribs.density);
    /*  reflects elastic collisions; heavier bullets make the lighter target yeet!  */

    tPtc.kinems.velocity.x += kPtc.kinems.velocity.x * transferRatio * 0.5f;
    tPtc.kinems.velocity.y += kPtc.kinems.velocity.y * transferRatio * 0.5f;

    // 4: Dislodge Check
    float tgtSpeeeeed = sqrtf((tPtc.kinems.velocity.x * tPtc.kinems.velocity.x)+(tPtc.kinems.velocity.y * tPtc.kinems.velocity.y));

    if (tgtSpeeeeed > 2.0f){
        tPtc.setKinetic(true);
        tPtc.setAwake(true);
    }

    // 5: Bounce/Reflect!
    kPtc.kinems.velocity.x *= -0.25f;
    kPtc.kinems.velocity.y *= -0.25f;
};

/*  MISC: self explanatory   */
void    ptcBox::boxUpdate   (int fCount){

    for (int idx = this->maxSize - 1; idx >= 0; idx--)     this->updatePtc(idx);
}
void    ptcBox::boxDraw     (bool debugMode){
    for (int i = 0; i < this->maxSize; i++){
        particle& ptc = this->particles[i];

        if (ptc.typeID == (uint8_t)ptcType::EMPTY) continue;

        const elementComponent& defin = elementRegistry[ptc.typeID];
        int sx = i % bounds;
        int sy = i / bounds;
        
        Color end_color = defin.color;
        if (ptc.isKinetic() && debugMode) end_color = ColorTint(end_color, GOLD  );

        DrawRectangle(
            this->xyBox.x + (sx * this->ptcScale), 
            this->xyBox.y + (sy * this->ptcScale),
            this->ptcScale,
            this->ptcScale,
            end_color);
    };

    if (!debugMode) return;
    // draws border!
    DrawRectangleLines(
        this->xyBox.x-1,
        this->xyBox.y-1,
        2+bounds*ptcScale,
        2+bounds*ptcScale,
        RED
    );
}
void    ptcBox::boxAdd      (int x, int y, ptcType element){
    int idxAdd = this->getPtcOffset((y * this->bounds) + x,0,0);

    if (idxAdd == -1 || idxAdd > this->maxSize) return;     // 1st CHECK: the index is within bounds

    bool isEraser =     (element == ptcType::EMPTY);
    bool isOccupied =   (this->particles[idxAdd].typeID != (uint8_t)ptcType::EMPTY);

    if (isOccupied && !isEraser) return;                    // 2nd CHECK: the particle does not override existing ones EXCEPT erasure

    particle& addPtc    = this->particles[idxAdd];
    addPtc.typeID       = (uint8_t)element;

    if (!(element == ptcType::EMPTY))    addPtc.setAwake(true);
    addPtc.kinems.resetK();

};
void    ptcBox::boxClear    (){
    for (int i = 0; i < maxSize; i++){ this->particles[i] = particle();};
};
void    ptcBox::boxDebug    (bool debugActivate){
    
    if(!debugActivate) return;
    
    for (int i = 0; i < maxSize; i++) {
        // Skip empty or sleeping particles
        if (particles[i].typeID == (uint8_t)ptcType::EMPTY) continue;
        if (!(particles[i].flags & FLAG_AWAKE)) continue; 

        // Calculate Position
        int sx = i % bounds;
        int sy = i / bounds;

        // Draw Green Outline for Awake Particles
        DrawRectangleLines(
            this->xyBox.x + (sx * this->ptcScale), 
            this->xyBox.y + (sy * this->ptcScale),
            this->ptcScale,
            this->ptcScale,
            GREEN
        );
    }
};