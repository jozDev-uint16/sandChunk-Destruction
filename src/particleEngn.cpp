#include "particleEngn.hpp"
#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "memory"
#include "vector"
#include "algorithm"
#include "random"

std::vector<elementCompo> elementRegistry;
void    initialElements(){
    elementRegistry.resize((int)elementID::_MAX_ID);

    elementRegistry[(int)elementID::POWDER] = {
        "placeholder powder", ColorBrightness(ColorContrast(GOLD,-0.35f),0.4f),
        {phaseCompo::phaseType::POWDER, 10, 0.35}
    };
}
int     randomSeedInt(int min, int max){
    std::random_device rando;
    std::mt19937 gen(rando());
    std::uniform_int_distribution<> out(min,max);

    return out(gen);
}

// SOLVERS
void    particleBox::ballisticSolver(int idx, const elementCompo define){

    auto& ptc = this->particles[idx];

    ptc.kinematics.velocity.x *= AIR_DRAG_BASE;
    ptc.kinematics.velocity.y += GRAVITY;

    float vectorSpd         = std::hypot(ptc.kinematics.velocity.x,ptc.kinematics.velocity.y);
    ptc.kinematics.inertia  = vectorSpd * define.phaseAttributes.mass;

    if  (vectorSpd < 1.0f){
        ptc.setState(false,IS_KINETIC);
        return;
    }  
    
    int     fallRay = (int)ceilf(vectorSpd);
    float   rayX    = ptc.kinematics.velocity.x/ (float)fallRay;
    float   rayY    = ptc.kinematics.velocity.y/ (float)fallRay;

    int     prevIdx = idx;

    for (int v = 0; v < fallRay; v++){
        
        // TODO: add da thing + convert rays -> multiple checkNeighs (holy shit...)

        const auto checkX = (rayX >= 0) ?   RIGHT : LEFT;
        const auto checkY = (rayY >= 0) ?   DOWN : UP;

        int nextIdx = getPtcOffset(getPtcOffset(prevIdx, checkX,(int)abs(rayX)), checkY,(int)abs(rayY));

        if (nextIdx == -1){
            ptc.setState(false,IS_KINETIC);
            ptc.kinematics.resetPhysics();

            ptc.setState(false,AWAKE);
            return;
        }

        this->particleSwap(prevIdx,nextIdx);
        prevIdx = nextIdx;
    }
};
void    particleBox::powderSolver   (int idx, const elementCompo define){
    auto& ptcs= this->particles;
    auto& ptc = ptcs[idx];

    bool flip = (randomSeedInt(1,100)>=50);

    int check[3] = {
        getPtcOffset(idx,DOWN,1),
        flip ? getPtcOffset(idx,DOWNLEFT,1) : getPtcOffset(idx,DOWNRIGHT,1),
        flip ? getPtcOffset(idx,DOWNRIGHT,1) : getPtcOffset(idx,DOWNLEFT,1)
    };

    ptc.kinematics.inertia += GRAVITY * define.phaseAttributes.mass;

    // first check (bottom)
    if (check[0] != -1){
        if (ptcs[check[0]].type == elementID::AIR){ 
            if (ptc.kinematics.inertia >= 1.0f){
                ptc.setState(true,IS_KINETIC);
                ptc.kinematics.velocity.y = ptc.kinematics.inertia;
            } 

            this->particleSwap(idx,check[0]);
            return;
        }
    }

    ptc.kinematics.inertia = 0;
    ptc.setState(false,IS_KINETIC);
    ptc.setState(false,AWAKE);
    return;
}; 

// CORE METHODS
int     particleBox::getPtcOffset    (int idx, checkBitmap target, uint8_t range){
    if (target == 0) return -1;

    int wide      = this->boundsPtcs;
    int checkFrom = std::__countr_zero((uint8_t)target);
    // ie DOWN has no zero trails, LEFT has 1, and so on

    auto dx = collideNeighbors [checkFrom][0] * range;
    auto dy = collideNeighbors [checkFrom][1] * range;

    int x = (idx % wide) + dx;
    int y = (idx / wide) + dy;

    if (x < 0 || y < 0)         return -1;
    if (x >= wide || y >= wide) return -1;

    return (y * wide + x);
};  
void    particleBox::particleUpdate  (int idx){
    auto& ptc           = this->particles[idx];
    const auto& define  = elementRegistry[(int)ptc.type];

    if (!ptc.checkState(AWAKE))     return;
    if (ptc.type == elementID::AIR) return;

    if (ptc.checkState(IS_KINETIC)){
        this->ballisticSolver(idx,define);
        return;
    }

    switch (define.phaseAttributes.phase){
        case phaseCompo::phaseType::STATIC: break;
        case phaseCompo::phaseType::POWDER: this->powderSolver(idx,define); break;
        case phaseCompo::phaseType::FLUID:  break;
        case phaseCompo::phaseType::VAPOR:  break;
        default:    return;
    }

};
void    particleBox::particleSwap    (int idxA, int idxB){
    auto& max       = this->maxPtcs;
    bool isValid    = (idxA < max) ||
                      (idxB < max);
    if (!isValid) return;

    auto& ptcs = this->particles;

    std::swap (ptcs[idxA],ptcs[idxB]);
    ptcs[idxA].setState(true,AWAKE);
    ptcs[idxB].setState(true,AWAKE);

};
void    particleBox::particleAdd     (int x, int y, elementID type){
    int idx = (y * this->boundsPtcs + x);
    if (idx == -1 || idx > this->maxPtcs)   return;

    auto& ptc = this->particles[idx];

    bool isErase    = (type == elementID::AIR);
    bool isTaken    = (ptc.type != elementID::AIR);
    if (isTaken && !isErase)                return;

    ptc = particle(type);
    ptc.kinematics.resetPhysics();

    //
};
void    particleBox::particleWake    (int idx){
    auto&   ptcs         = this->particles;

    for (uint8_t n = 0; n < 4; n++){
        uint8_t check   = (uint8_t)2^n;
        int pick        = this->getPtcOffset (idx,(checkBitmap)check,1);
    
        if (pick == -1) continue;

        ptcs[pick].setState(true,AWAKE);   
    }
};
// BOX METHODS
void    particleBox::boxUpdate       (){
    const auto& max = this->maxPtcs - 1;

    for (int i = max; i >= 0; i--) this->particleUpdate(i);
};
void    particleBox::boxRender       (bool debug, int zoom){
    const auto& max     = this->maxPtcs - 1;
    auto&       scale   = this->scalePtcs;
    const auto& screen  = this->boxScreenPos;

    auto    boxPixels     = (Color*)this->ptcTxtr.data;
    auto&   ptcs          = this->particles;

    ImageClearBackground(&ptcTxtr,BLANK);

    //  DRAW INTO IMAGE FOR PIXELS
    for (int i = max; i >= 0; i--){
        auto& define = elementRegistry[(uint8_t)ptcs[i].type];

        if (ptcs[i].type == elementID::AIR) continue;

        Color drawC = define.color;
        if (ptcs[i].checkState(IS_KINETIC)) drawC = ColorTint(drawC, RAYWHITE);

        boxPixels[i] = drawC;
    }

    scale += zoom;

    //  UPDATE TO GPU FOR DRAWING
    UpdateTexture(this->ptcTxtrDrawn,boxPixels);
    DrawTextureEx(this->ptcTxtrDrawn,
        screen, 
        0.0f,
        scale,
        WHITE
    );

    //  DEBUGGING VISUALS: Border, Particle Visuals
    if (debug) this->boxDebugs();
   
};
void    particleBox::boxReset        (bool activate){
    if  (!activate) return;
    for (int i = 0; i < this->maxPtcs; i++) this->particles[i] = particle();
};
void    particleBox::boxDebugs       (){
    const auto& max     = this->maxPtcs - 1;
    auto&       scale   = this->scalePtcs;
    const auto& screen  = this->boxScreenPos;
    
    auto&   ptcs          = this->particles;

    DrawText(
        "(!) DEBUG MODE", (GetScreenWidth()/2)-100,0,
        20, RAYWHITE
    );
    DrawRectangleLines(
        screen.x-1,
        screen.y-1,
        2 + scale * this->boundsPtcs,
        2 + scale * this->boundsPtcs,
        ColorTint(RED,RAYWHITE)
    );

    for (int i = max; i >= 0; i--){

        if (ptcs[i].type == elementID::AIR) continue;
        if (!ptcs[i].checkState(AWAKE)) continue;

        int sx = i % this->boundsPtcs;
        int sy = i / this->boundsPtcs;

        DrawRectangleLines(
            sx + this->boxScreenPos.x,
            sy + this->boxScreenPos.y, scale, scale,
            RED
        );

        if (!(ptcs[i].kinematics.inertia < 1)){
            DrawRectangleLines(
            sx + this->boxScreenPos.x + this->boundsPtcs,
            sy + this->boxScreenPos.y, scale, scale,
            GOLD
        );
        }
        if (ptcs[i].checkState(IS_KINETIC)){
            DrawRectangleLines(
            sx + this->boxScreenPos.x + (this->boundsPtcs * 2),
            sy + this->boxScreenPos.y, scale, scale,
            GREEN
        );
        }
        if (ptcs[i].checkState(ASLEEP)){
            DrawRectangleLines(
            sx + this->boxScreenPos.x,
            sy + this->boxScreenPos.y + (this->boundsPtcs * 2), 
            scale, scale, BLUE
        );
        }
    }
};