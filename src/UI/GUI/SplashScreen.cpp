#include "SplashScreen.h"

SplashScreen::SplashScreen() : DisplayItem("Splashscreen"){}

void SplashScreen::draw(){
    startParameters();
}

void SplashScreen::startParameters(){
    //Field of view in degrees.
    double fov = 90.0;
    // Near and far values
    double n = 0.1, f = 100;
    double aspectRatio = width*1.0/height;
    double scale = tan(fov * PI / 360.0) * n;
    double r = aspectRatio*scale; //Right
    double l = -r;      //Left
    double t = scale;   //Top
    double b = -t;      //Bottom

    double projMatrix[4][4];
    projMatrix[0][0] = 2 * n / (r - l); 
    projMatrix[0][1] = 0; 
    projMatrix[0][2] = 0; 
    projMatrix[0][3] = 0; 
 
    projMatrix[1][0] = 0; 
    projMatrix[1][1] = 2 * n / (t - b); 
    projMatrix[1][2] = 0; 
    projMatrix[1][3] = 0; 
 
    projMatrix[2][0] = (r + l) / (r - l); 
    projMatrix[2][1] = (t + b) / (t - b); 
    projMatrix[2][2] = -(f + n) / (f - n); 
    projMatrix[2][3] = -1; 
 
    projMatrix[3][0] = 0; 
    projMatrix[3][1] = 0; 
    projMatrix[3][2] = -2 * f * n / (f - n); 
    projMatrix[3][3] = 0; 

    double th = millis()/1000.0;
    double rotMatrix[4][4] = {{cos(th), 0, sin(th), 0},
                                {0, 1, 0, 0},
                                {-sin(th), 0, cos(th), 0},
                                {0, 0, 0, 1}};

    double scaleMatrix[4][4] = {{5, 0, 0, 0},
                                {0, 5, 0, 0},
                                {0, 0, 5, 0},
                                {0, 0, 0, 1}};

    double worldToCamera[4][4] = {{1, 0, 0, 0},
                                {0, 1, 0, 0},
                                {0, 0, 1, 0},
                                {0, 0, -20, 1}};

    tft->fillScreen(TFT_BLACK);
    uint16_t totalPoints = sizeof(vertex)/sizeof(vertex[0]);
    uint16_t projectedPoints[totalPoints][2];
    for(uint16_t index = 0; index < totalPoints; index++){
        double transf[3];
        multMatrix(vertex[index], scaleMatrix, transf);
        multMatrix(transf, rotMatrix, transf);
        multMatrix(transf, worldToCamera, transf);

        double projVertex[3];
        multMatrix(transf, projMatrix, projVertex);

        //This vertex will be out of the screen
        if (projVertex[0] < -aspectRatio || projVertex[0] > aspectRatio || projVertex[1] < -1.0 || projVertex[1] > 1.0) continue; 
    
        uint16_t x = min(width-1.0, (projVertex[0]+1)*width/2.0);
        uint16_t y = min(height-1.0, (1.0 - (projVertex[1]+1.0)/2.0)*height);
        projectedPoints[index][0] = x;
        projectedPoints[index][1] = y;

        tft -> drawSpot(x, y, 1, TFT_RED, TFT_RED);
        //Serial.printf("(%f,%f,%f) -> (%d, %d)\n", vertex[index][0],vertex[index][1],vertex[index][2],x,y);
    }

    uint16_t totalFaces = sizeof(faces)/sizeof(faces[0]);
    for(uint16_t index = 0; index < totalFaces; index++){
        for(uint8_t p = 0; p < 4; p++){
            uint16_t f0 = faces[index][p==0 ? 3 : p-1] - 1;
            uint16_t f1 = faces[index][p] - 1;

            uint16_t x0 = projectedPoints[f0][0];
            uint16_t y0 = projectedPoints[f0][1];
            uint16_t x1 = projectedPoints[f1][0];
            uint16_t y1 = projectedPoints[f1][1];
            tft -> drawLine(x0, y0, x1, y1, TFT_WHITE);
        }
    }

}

void SplashScreen::multMatrix(double inCart[3], double matrix[4][4], double outCart[3]){
    double in[4] = {inCart[0], inCart[1], inCart[2], 1.0};
    double out[4] = {0,0,0,0};
    for(uint8_t i = 0; i < 4; i++){
        for(uint8_t j = 0; j < 4; j++){
            out[i] += in[j]*matrix[j][i];
        }
    }
    // From homogeneous to cartessian
    if(out[3] != 1.0){
        out[0] /= out[3];
        out[1] /= out[3];
        out[2] /= out[3];
    }
    outCart[0] = out[0];
    outCart[1] = out[1];
    outCart[2] = out[2];
}