#include <lcdutils.h>
#include <msp430.h>
#include <p2switches.h>
#include "libTimer.h"
//#include "buzzer.h"
#include <lcddraw.h>
#include "shape.h"

#define GREEN_LED BIT6

int movePlayer = 0;//permission to move paddle

AbRect rectangle = {abRectGetBounds, abRectCheck, {10,10}}; //square

AbRectOutline fieldOutline = {/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2 - 5, screenHeight/2 - 5}
};

Layer layer1 = {
  (AbShape *)&rectangle,
  {(screenWidth/2), (screenHeight/2)},      /* position */
  {0,0}, {0,0},                             /* last & next pos */
  COLOR_WHITE,
  0,
};

Layer fieldLayer = {        /* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},                 /* last & next pos */
  COLOR_BLACK,
  &layer1
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer ml0 = {&layer1, {0,0}, 0};

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);           /**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);         /**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
        bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
    Vec2 pixelPos = {col, row};
    u_int color = bgColor;
    Layer *probeLayer;
    for (probeLayer = layers; probeLayer;
         probeLayer = probeLayer->next) { /* probe all layers, in order */
      if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
        color = probeLayer->color;
        break;
      } /* if probe check */
    } // for checking all layers at col, row
    lcd_writeColor(color);
      } // for col
    } // for row
  } // for moving layer being updated
}

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
      (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
    int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
    newPos.axes[axis] += (2*velocity);
      } /**< if outside of fence */
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

//Check which button is pressed
void checkPlayer(u_int switches){
unsigned char S1 = (switches & 1) ? 0 : 1;//First switch
unsigned char S2 = (switches & 2) ? 0 : 1;//Second switch
unsigned char S4 = (switches & 8) ? 0 : 1;//Third switch

static int num = 0;

static int x = 0;//Change horizontal velocity

if(S1 && num != 0 && movePlayer != 1){ //Moves left
  x = -5;
}
else if(S4 && num != 0 && movePlayer != 1){ //Moves right
  x = 5;
}
else{ //Don't move
  x = 0;
}

Vec2 newVelocity = {x, 0};
(&ml0)->velocity = newVelocity;

    if(S2 && num == 0){ //Cover up starting screen
           drawString8x12(10,10, "MELINA", COLOR_BLACK, COLOR_AQUAMARINE);
           Vec2 newVelocity1 = {3, -3}; //Moving triangle velocity
           (&ml0)->velocity = newVelocity1;
           num++; //Only release once
    }
}

u_int bgColor = COLOR_HOT_PINK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */
Region fieldFence;              /**< fence around playing field  */

int main(void)
{

    configureClocks();
    lcd_init();     //Initialize LCD display
    shapeInit();
    p2sw_init(15);  //Initialize all switches
    shapeInit();
    layerInit(&fieldLayer);
    layerDraw(&fieldLayer);
    layerGetBounds(&fieldLayer, &fieldFence);
    enableWDTInterrupts();      /**< enable periodic interrupt */
    or_sr(0x8);                 /**< GIE (enable interrupts) */
    drawString8x12(15,10, "HELLO WORLD!", COLOR_BLACK, COLOR_AQUAMARINE);
    //buzzer_init();
    u_int inter;
    for(;;) {
      inter = p2sw_read();
      while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
        P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
        or_sr(0x10);        /**< CPU OFF */
      }
      P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
      redrawScreen = 0;
      checkPlayer(inter);       //Constantly check which switch is down to move
      movLayerDraw(&ml0, &layer1);
    }
    return 0;
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;             /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &fieldFence);
      redrawScreen = 1;
      count = 0;
  }
  P1OUT &= ~GREEN_LED;          /**< Green LED off when cpu off */
}
