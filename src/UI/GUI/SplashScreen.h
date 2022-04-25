#ifndef SPLASHSCREEN_h
#define SPLASHSCREEN_h

#include "DisplayItem.h"

class SplashScreen : public DisplayItem{

    public:
    double vertex[8][3] = {{1,1,-1},{1,-1,-1},{1,1,1},{1,-1,1},
                                    {-1,1,-1},{-1,-1,-1},{-1,1,1},{-1,-1,1}};
    uint16_t faces[6][4] = {{1,5,7,3},{4,3,7,8},{8,7,5,6},{6,2,4,8},{2,1,3,4},{6,5,1,2}};

    SplashScreen();

    void draw() override;
    void startParameters();
    void multMatrix(double in[4], double matrix[4][4], double out[4]);

};

#endif