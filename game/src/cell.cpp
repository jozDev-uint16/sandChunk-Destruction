#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include "memory"
#include "algorithm"


#include "cell.hpp"

elementRecipe updateManager[MAX_TYPES];
Color elementPalette[MAX_TYPES]     = {};

behaviorComp updateList[MAX_TYPES]  = {};     // INITIALIZE MEMORY -> also makes them null!
renderComp renderList[MAX_TYPES]    = {};

/*  place logic strategies here */
float randomHash(std::size_t index) {
    // The "Magic Numbers" (12.9898, 78.233, 43758.5453) are standard constants 
    // used in graphics to generate static noise without visible patterns.
    
    float x = (float)index;
    
    // We calculate a sine wave, but multiply the result by a huge number.
    float noise = sinf(x * 12.9898f + 78.233f) * 43758.5453f;
    
    // Return only the fractional part (e.g., 123.456 -> 0.456)
    return noise - floorf(noise); 
};




//
/* START OF CELLDOMAIN */
//
std::size_t cellDomain::readIndex   (std::size_t x, std::size_t y) const {
        return y * (std::size_t) xBounds + x;
    };
bool cellDomain::checkBounds        (std::size_t x, std::size_t y) const {
        return (x < xBounds && y < yBounds);
    };

renderComp& cellDomain::readRender  (std::size_t i) { 
        return renderCompCells[i];
    };

behaviorComp& cellDomain::readBehavior  (std::size_t i) { 
        return behaviorCompCells[i];
    };
//

std::size_t cellDomain::getOffset (std::size_t cell, std::size_t dx, std::size_t dy){
    std::size_t current_x = (std::size_t)(cell % xBounds);
    std::size_t current_y = (std::size_t)(cell / xBounds);

    // 2. Perform signed arithmetic
    std::size_t newx = current_x + dx;
    std::size_t newy = current_y + dy;

    if (newx < 0 || newx >= (std::size_t)xBounds || newy < 0 || newy >= (std::size_t)yBounds) return -1;
    //  INVALID if out of bounds

    return (std::size_t)((newy * xBounds) + newx);
};

void cellDomain::cellSwap (std::size_t cellA, std::size_t cellB){
    bool ValidA = (cellA < this->maxBounds);
    bool ValidB = (cellB < this->maxBounds);

    if (!ValidA || !ValidB) return;

    std::swap(this->behaviorCompCells[cellA],this->behaviorCompCells[cellB]);
    std::swap(this->renderCompCells[cellA],this->renderCompCells[cellB]);
        // add below any additionals
     
};

void cellDomain::cellAdd (std::size_t cx, std::size_t cy, cName id){

    if (!(this->checkBounds(cx,cy)|| id >= MAX_TYPES)) return; 

    std::size_t cell = this->readIndex(cx,cy);

    bool isEraser = (id == NONE);
    bool isOccupied = (behaviorCompCells[cell].type != NONE);

    // Only return (fail) if it's occupied AND WE ARE NOT ERASING
    if (isOccupied && !isEraser) return;

    // Copy the provided Render Component into the grid
    this->renderCompCells[cell] =   renderList[id];
    this->behaviorCompCells[cell] = updateList[id];

    // Ensure the new particle is marked active for the next update cycle
    if (id != NONE) {
        behaviorCompCells[cell].active = true;
        return;
    } 
    // If erasing, ensure the cell is deactivated immediately
    behaviorCompCells[cell].active = false;
};

void cellDomain::cellUpdate(std::size_t x, std::size_t y){
    std::size_t cell = this->readIndex(x,y);
    behaviorComp& bComp = this->behaviorCompCells[cell];

    if (bComp.type == NONE) return;            // if inactive or empty (NONE) cell

    const elementRecipe& recipe = updateManager[bComp.type];
    if (recipe.count == 0) return;

    for (uint8_t k = 0; k < recipe.count; k++)    {
        /* code */
        behaviorLogic nowlogic = recipe.bSteps[k];

        std::size_t tcell = nowlogic(cell, this);

        if (bComp.active && cell == tcell){
            this->behaviorCompCells[tcell].active = false;
            break;
        };

        this->behaviorCompCells[tcell].active = true;
        this->cellSwap  (cell,tcell);
        

        cell = tcell;

    };
};
//  just extracted logic

void cellDomain::domainUpdate   (int frameCount){
    bool scanLR = (frameCount % 2 == 0);

    for (int y = (int)this->yBounds - 1; y >= 0; y--){

        if (scanLR){
            for (int x = 0; x < (int)this->xBounds; x++)    cellUpdate(x,y);
            continue;
        };

        for (int x = (int)this->xBounds - 1; x >= 0; x--)   cellUpdate(x,y);
        
    };
};
void cellDomain::domainRender   (){
    
    for (std::size_t i = 0; i < this->maxBounds; i++){
        /* code */
        const renderComp& rComp = this->renderCompCells[i];
        const behaviorComp& bComp = this->behaviorCompCells[i];

        if (bComp.type == NONE) continue;

        std::size_t x = i % xBounds;
        std::size_t y = i / xBounds;
        Vector2 screenDomain = {
            this->boxPos.x + (x * this->cellSize), 
            this->boxPos.y + (y * this->cellSize)
        };

        Color end_color = rComp.color;

        if (rComp.post != nullptr)  end_color = rComp.post(i,this);     // if shading exists, overwrite the end color

        DrawRectangle(
            screenDomain.x,
            screenDomain.y,
            cellSize,
            cellSize,
            end_color);
    };
};
void cellDomain::domainReset    (){
    for (std::size_t i = 0; i < this->maxBounds; i++){
        this->renderCompCells[i] = renderComp();
        this->behaviorCompCells[i] = behaviorComp();
    }
};
//
/*  END OF CELLDOMAIN  */
//

std::size_t basicPowder(std::size_t cell, cellDomain* domain){
    uint8_t mask = ((1<<0) | (1<<1) | (1<<2));  //  collide mask will be unique to ALL behaviors
    cName valids = NONE;                        //  Valids are cell names that are allowed to swap to (mainly NONE)

    bool flip = (GetRandomValue(0, 1) == 1);
    const uint8_t* checkOrder = flip ? ORDER_SWAPPED : ORDER_NORMAL;

    for (uint8_t i = 0; i < 8; i++){
        
        uint8_t k = checkOrder[i];
        
        if (mask & (1 << k)) {
            int dx = NEIGHBORS[k].dx;
            int dy = NEIGHBORS[k].dy;

            if ((int)(domain->getOffset(cell, dx, dy)) == -1) continue;

            std::size_t target = domain->getOffset(cell, dx, dy);

            if (domain->readBehavior((std::size_t)target).type == valids) return target;
        };      
    };
    return cell;    // if all else fails (would be factored out soon)
};

std::size_t basicStatic(std::size_t cell, cellDomain* domain){
    cName valids = NONE;
    
    int cellx = NEIGHBORS[3].dx + 1;
    int celly = NEIGHBORS[3].dy;

    if ((int)(domain->getOffset(cell, cellx, celly) == -1)) return cell;

    std::size_t target = domain->getOffset(cell, cellx, celly);

    if (domain->readBehavior((std::size_t)target).type == valids) return target;
    
    return cell;
};

Color grainyShading(std::size_t cell, cellDomain* domain){

    //  x is hue,   y is sat,   z is value
    Color &base = domain->renderCompCells[cell].color;
    Vector3 range = domain->renderCompCells[cell].dHSV; 
    Vector3 hsv = ColorToHSV(base);

    // --- NOISE GENERATION ---
    
    // Get a 0.0 to 1.0 value
    float noise01 = randomHash(cell); 
    // Map it to range -1.0 to 1.0 (so we can darken OR lighten)
    float noise = (noise01 * 2.0f) - 1.0f; 

    // ------------------------

    // Apply Hue (Wrapped)
    hsv.x += (range.x * noise * 10.0f); 
    if (hsv.x < 0.0f) hsv.x += 360.0f;
    if (hsv.x >= 360.0f) hsv.x -= 360.0f;

    // Apply Saturation
    hsv.y += (range.y * noise);
    hsv.y = fmaxf(0.0f, fminf(1.0f, hsv.y));

    // Apply Value
    hsv.z += (range.z * noise);
    hsv.z = fmaxf(0.0f, fminf(1.0f, hsv.z));

    Color result = ColorFromHSV(hsv.x, hsv.y, hsv.z);
    result.a = base.a;

    return result;
};

//
/*  Below are added uniques (ie components for a unique cell)   */
//

void initElementRecipe(){
    for (uint16_t i = 0; i < MAX_ELEM_STEPS; i++)   updateManager[i].count = 0;

    updateManager[SAND] = {1, 15,
        {   basicPowder, nullptr, nullptr, nullptr }
    };
    updateManager[GRAVEL] = {1, 20,
        {   basicPowder, nullptr, nullptr, nullptr }
    };
    updateManager[WOOD] = {1, 10,
        {   basicStatic, nullptr, nullptr, nullptr }
    };

};
void initCellPalette(){
/*  List of Colors for each element (may be phased out) */
    elementPalette[SAND] = (Color){237, 211, 157, 255};
    elementPalette[GRAVEL] = (Color){95, 94, 93, 255};
    elementPalette[WOOD] = (Color){95, 62, 42, 255};


/*  Contingent list of particle behavior comps (use cName as index!)*/
    updateList[NONE] =     behaviorComp();
    renderList[NONE] =     renderComp();

    updateList[SAND] =     behaviorComp(SAND);
    renderList[SAND] =       renderComp(elementPalette[SAND],((Vector3){0.0f,0.2f,0.0f}), grainyShading);

    updateList[GRAVEL] =   behaviorComp(GRAVEL);
    renderList[GRAVEL] =     renderComp(elementPalette[GRAVEL],((Vector3){0.0f,0.2f,0.1f}), grainyShading);

    updateList[WOOD] =   behaviorComp(WOOD);
    renderList[WOOD] =     renderComp(elementPalette[WOOD],((Vector3){0.0f,0.2f,0.3f}), grainyShading);
    // add here
    //  updateList[ ]
    //  renderList[ ]


};



/*      TODO:   behaviorComp param will have source from another list! the updateManager*/







