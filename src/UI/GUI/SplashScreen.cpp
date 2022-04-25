#include "SplashScreen.h"

SplashScreen::SplashScreen() : DisplayItem("Splashscreen"){
    // First fill the edges vector. It makes sure that the edges aren't repeated.
    uint16_t faceCount = sizeof(faces)/sizeof(faces[0]);
    for(uint16_t i = 0; i < faceCount; i++){
        for(uint8_t j = 0; j < 4; j++){
            uint16_t v0 = faces[i][j==0 ? 3 : j-1];
            uint16_t v1 = faces[i][j];
            if(v0 > v1){
                uint16_t temp = v0;
                v0 = v1;
                v1 = temp;
            }
            bool edgeProcessed = false;
            for(std::array<uint16_t, 2> arr : edges){
                if(arr[0] == v0 && arr[1] == v1){
                    edgeProcessed = true;
                    break;
                } 
            }
            if(!edgeProcessed){
                std::array<uint16_t, 2> edge = {v0,v1};
                edges.push_back(edge);
            }
        }
    }
}

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

    // Projection matrix, from world to screen
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
    // Rotate around the Y axis.
    double rotMatrix[4][4] = {{cos(th), 0, sin(th), 0},
                                {0, 1, 0, 0},
                                {-sin(th), 0, cos(th), 0},
                                {0, 0, 0, 1}};

    double scaleMatrix[4][4] = {{5, 0, 0, 0},
                                {0, 5, 0, 0},
                                {0, 0, 5, 0},
                                {0, 0, 0, 1}};

    double translationMatrix[4][4] = {{1, 0, 0, 0},
                                {0, 1, 0, 0},
                                {0, 0, 1, 0},
                                {0, 0, -15, 1}};

    tft->fillScreen(TFT_BLACK);

    // Calculates the projected points to the screen.
    uint16_t totalPoints = sizeof(vertex)/sizeof(vertex[0]);
    uint16_t projectedPoints[totalPoints][2];
    for(uint16_t index = 0; index < totalPoints; index++){
        //In this order: scale, rotate and translate the object.
        double transf[3];
        multMatrix(vertex[index], scaleMatrix, transf);
        multMatrix(transf, rotMatrix, transf);
        multMatrix(transf, translationMatrix, transf);

        double projVertex[3];
        multMatrix(transf, projMatrix, projVertex);

        //This vertex will be out of the screen
        if (projVertex[0] < -aspectRatio || projVertex[0] > aspectRatio || projVertex[1] < -1.0 || projVertex[1] > 1.0) continue; 
    
        uint16_t x = min(width-1.0, (projVertex[0]+1)*width/2.0);
        uint16_t y = min(height-1.0, (1.0 - (projVertex[1]+1.0)/2.0)*height);
        projectedPoints[index][0] = x;
        projectedPoints[index][1] = y;

        //Serial.printf("(%f,%f,%f) -> (%d, %d)\n", vertex[index][0],vertex[index][1],vertex[index][2],x,y);
    }

    //Calculates the faces that aren't seen in the viewport.
    uint16_t totalFaces = sizeof(faces)/sizeof(faces[0]);
    bool drawEdges[edges.size()];
    std::fill_n(drawEdges, edges.size(), false);
    for(uint16_t index = 0; index < totalFaces; index++){
        double norm[3]; //Normal of the face
        multMatrix(normals[index], rotMatrix, norm);
        double toFace[3]; //Vector to any vertex in the face from the camera.
        multMatrix(vertex[faces[index][0]-1], scaleMatrix, toFace);
        multMatrix(toFace, rotMatrix, toFace);
        multMatrix(toFace, translationMatrix, toFace);

        //If the angle between the camera and the face is equal or greater than 90º,
        //that is v·w > 0, then it's not facing the camera.
        double dot = norm[0]*toFace[0] + norm[1]*toFace[1] + norm[2]*toFace[2];
        if(dot > 0) continue; //If not visible continue

        //If visible, assign the sides to also be visible.
        for(uint8_t j = 0; j < 4; j++){
            uint16_t v0 = faces[index][j==0 ? 3 : j-1];
            uint16_t v1 = faces[index][j];
            if(v0 > v1){
                uint16_t temp = v0;
                v0 = v1;
                v1 = temp;
            }
            uint16_t drawIndex = 0;
            for(std::array<uint16_t, 2> arr : edges){
                if(arr[0] == v0 && arr[1] == v1){
                    drawEdges[drawIndex] = true;
                    break;
                }
                drawIndex++;
            }
        }
    }

    //First, draw the hidden edges.
    uint16_t drawIndex = 0xFFFF;
    for(bool d : drawEdges){
        drawIndex++;
        if(d) continue;

        std::array<uint16_t, 2> edge = edges[drawIndex];
        uint16_t x0 = projectedPoints[edge[0]-1][0];
        uint16_t y0 = projectedPoints[edge[0]-1][1];
        uint16_t x1 = projectedPoints[edge[1]-1][0];
        uint16_t y1 = projectedPoints[edge[1]-1][1];

        //Draws a dashed line of 5 segments between the vertices.
        double deltaX = x1*1.0 - x0;
        double deltaY = y1*1.0 - y0;
        for(double i = 0; i < 1; i += 0.222){
            double newX0 = x0 + deltaX*i;
            double newY0 = y0 + deltaY*i;
            double newX1 = x0 + deltaX*(i+0.111);
            double newY1 = y0 + deltaY*(i+0.111);
            tft -> drawLine(newX0, newY0, newX1, newY1, 0x4A69);
        }
    }

    //Now draws the visible edges.
    drawIndex = 0xFFFF;
    for(bool d : drawEdges){
        drawIndex++;
        if(!d) continue;

        std::array<uint16_t, 2> edge = edges[drawIndex];
        uint16_t x0 = projectedPoints[edge[0]-1][0];
        uint16_t y0 = projectedPoints[edge[0]-1][1];
        uint16_t x1 = projectedPoints[edge[1]-1][0];
        uint16_t y1 = projectedPoints[edge[1]-1][1];
        tft -> drawLine(x0, y0, x1, y1, TFT_WHITE);
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