#include "SplashScreen.h"

SplashScreen::SplashScreen() : DisplayItem("Splashscreen"){
    if(fillPolygons) return;
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
    TFT_eSprite spr = TFT_eSprite(tft);
    spr.createSprite(w, h);
    spr.fillSprite(TFT_BLACK);
    startParameters(spr);
    spr.pushSprite((width-w)/2, 0);
}

void SplashScreen::startParameters(TFT_eSprite &spr){
    //Field of view in degrees.
    double fov = 60.0;
    // Near and far values
    double n = 0.1, f = 100;
    double aspectRatio = w*1.0/h;
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
    double rotMatrix1[4][4] = {{cos(th), 0, sin(th), 0},
                                {0, 1, 0, 0},
                                {-sin(th), 0, cos(th), 0},
                                {0, 0, 0, 1}};
    double rotMatrix2[4][4] = {{cos(th), sin(th), 0, 0},
                            {-sin(th), cos(th), 0, 0},
                            {0,0,1,0},
                            {0, 0, 0, 1}};
    double rotMatrix[4][4];
    multMatrix(rotMatrix1, rotMatrix2, rotMatrix);

    double scaleMatrix[4][4] = {{5, 0, 0, 0},
                                {0, 5, 0, 0},
                                {0, 0, 5, 0},
                                {0, 0, 0, 1}};

    double translationMatrix[4][4] = {{1, 0, 0, 0},
                                    {0, 1, 0, 0},
                                    {0, 0, 1, 0},
                                    {0, 0, -25, 1}};

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
        multMatrixHomogeneous(transf, projMatrix, projVertex);

        //This vertex will be out of the screen
        if (projVertex[0] < -aspectRatio || projVertex[0] > aspectRatio || projVertex[1] < -1.0 || projVertex[1] > 1.0) continue; 
    
        uint16_t x = min(w-1.0, (projVertex[0]+1)*w/2.0);
        uint16_t y = min(h-1.0, (1.0 - (projVertex[1]+1.0)/2.0)*h);
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
        if(dot > 0) continue; //If not visible, continue

        if(fillPolygons){
            double sunDot = norm[0]*sun[0] + norm[1]*sun[1] + norm[2]*sun[2];
            double shadowFactor = max(0.3, (1-sunDot)/2);

            uint16_t totalEdges = sizeof(faces[index])/sizeof(faces[index][0]);
            uint16_t ind0 = faces[index][0] - 1;
            uint16_t vert0x = projectedPoints[ind0][0];
            uint16_t vert0y = projectedPoints[ind0][1];
            //The number of triangles inside a polygon is totalEdges-2
            for(uint16_t tr = 0; tr < totalEdges-2; tr++){
                uint16_t ind1 = faces[index][tr+1] - 1;
                uint16_t vert1x = projectedPoints[ind1][0];
                uint16_t vert1y = projectedPoints[ind1][1];
                
                uint16_t ind2 = faces[index][tr+2] - 1;
                uint16_t vert2x = projectedPoints[ind2][0];
                uint16_t vert2y = projectedPoints[ind2][1];
                
                uint16_t color = tft -> color565(128*shadowFactor, 128*shadowFactor, 128*shadowFactor);
                spr.fillTriangle(vert0x, vert0y, vert1x, vert1y, vert2x, vert2y, color);
            }
        }else{
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
    }

    if(fillPolygons) return;

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
            spr.drawLine(newX0, newY0, newX1, newY1, 0x4A69);
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
        spr.drawLine(x0, y0, x1, y1, TFT_WHITE);
    }
}

// inVector * matrix = outVector
void SplashScreen::multMatrix(double inVector[3], double matrix[4][4], double outVector[3]){
    double in[4] = {inVector[0], inVector[1], inVector[2], 1.0};
    double out[4] = {0,0,0,0};
    for(uint8_t i = 0; i < 4; i++){
        for(uint8_t j = 0; j < 4; j++){
            out[i] += in[j]*matrix[j][i];
        }
    }
    outVector[0] = out[0];
    outVector[1] = out[1];
    outVector[2] = out[2];
}

void SplashScreen::multMatrix(double matrixA[4][4], double matrixB[4][4], double outMatrix[4][4]){
    for(uint8_t i = 0; i < 4; i++) 
        for(uint8_t j = 0; j < 4; j++)
            outMatrix[i][j] = 0;

    for(uint8_t h = 0; h < 4; h++)
        for(uint8_t i = 0; i < 4; i++) 
            for(uint8_t j = 0; j < 4; j++)
                outMatrix[h][i] += matrixA[h][j]*matrixB[j][i];
}

void SplashScreen::multMatrixHomogeneous(double inVector[3], double matrix[4][4], double outVector[3]){
    double in[4] = {inVector[0], inVector[1], inVector[2], 1.0};
    double out[4] = {0,0,0,0};
    for(uint8_t i = 0; i < 4; i++){
        for(uint8_t j = 0; j < 4; j++){
            out[i] += in[j]*matrix[j][i];
        }
    }

    //Clipping from homogeneous clip space
    if(out[3] < 0) Serial.println("Homogeneous w in vector out, cannot be less than 1!");
    for(uint8_t i = 0; i < 3; i++){
        if(out[i] < -out[3]) out[i] = -out[3];
        else if(out[i] > out[3]) out[i] = out[3];
    }

    // From homogeneous to cartessian
    if(out[3] != 1.0){
        out[0] /= out[3];
        out[1] /= out[3];
        out[2] /= out[3];
    }
    outVector[0] = out[0];
    outVector[1] = out[1];
    outVector[2] = out[2];
}