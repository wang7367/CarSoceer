/** CSci-4611 Assignment 2:  Car Soccer
 */

#include "car_soccer.h"
#include "config.h"

CarSoccer::CarSoccer() : GraphicsApp(1024,768, "Car Soccer") {
    // Define a search path for finding data files (images and shaders)
    searchPath_.push_back(".");
    searchPath_.push_back("./data");
    searchPath_.push_back(DATA_DIR_INSTALL);
    searchPath_.push_back(DATA_DIR_BUILD);
}

CarSoccer::~CarSoccer() {
}


Vector2 CarSoccer::joystick_direction() {
    Vector2 dir;
    if (IsKeyDown(GLFW_KEY_LEFT))
        dir[0]--;
    if (IsKeyDown(GLFW_KEY_RIGHT))
        dir[0]++;
    if (IsKeyDown(GLFW_KEY_UP))
        dir[1]++;
    if (IsKeyDown(GLFW_KEY_DOWN))
        dir[1]--;

    return dir;
  
}


void CarSoccer::OnSpecialKeyDown(int key, int scancode, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        // Here's where you could call some form of launch_ball();
        ball_.launch_ball();
    }
}


void CarSoccer::UpdateSimulation(double timeStep) {
    // Here's where you shound do your "simulation", updating the positions of the
    // car and ball as needed and checking for collisions.  Filling this routine
    // in is the main part of the assignment.

    //rotate the car and move forward
       turnRate_ -= joystick_direction()[0];

        m = Matrix4::RotationY(GfxMath::ToRadians(turnRate_));

        float carSpeed = car_.speed() - joystick_direction()[1];
        carSpeed = carSpeed + drag_ * carSpeed;

        Vector3 forwardSpeed = Vector3(0, 0, carSpeed);
        Vector3 carVel = m * forwardSpeed;
        car_.set_speed(carSpeed);

    Point3 carPos = car_.position() + carVel*timeStep;
    //check if the car is outside of the pitch, if so, let it stay in.
    if (carPos.x() > 40 - car_.collision_radius()) {
        carPos[0] = 40 - car_.collision_radius();
    }
    if (carPos.x() < -40 + car_.collision_radius()) {
        carPos[0] = -40 + car_.collision_radius();
    }

    if (carPos.z() > 50 - car_.collision_radius()) {
        carPos[2] = 50 - car_.collision_radius();
    }
    if (carPos.z() < -50 + car_.collision_radius()) {
        carPos[2] = -50 + car_.collision_radius();
    }


    Vector3 ballVel= ball_.velocity() + gravity_ * timeStep;
    ballVel = ballVel + airDrag_ * ballVel;
    Point3 ballPos = ball_.position() + ballVel * timeStep;
    //check if ball is outside of the pitch, if so, let it stay in
    if (ballPos.y() > 35 - ball_.radius()) {
        ballPos[1]= 35 - ball_.radius();
        ballVel[1]*= -COR_;
    }
    if (ballPos.y() < ball_.radius()) {
        ballPos[1] = ball_.radius();
        ballVel[1] *= -COR_;
    }
    if (ballPos.x() > 40 - ball_.radius()) {
        ballPos[0] = 40 - ball_.radius();
        ballVel[0] *= -COR_;
    }
    if (ballPos.x() < -40 + ball_.radius()) {
        ballPos[0] = -40 + ball_.radius();
        ballVel[0] *= -COR_;
    }

    if (ballPos.z() > 50 - ball_.radius()) {
        ballPos[2] = 50 - ball_.radius();
        ballVel[2] *= -COR_;
    }
    if (ballPos.z() < -50 + ball_.radius()) {
        ballPos[2] = -50 + ball_.radius();
        ballVel[2] *= -COR_;
    }
 
    float distance = sqrt(pow(ballPos[0] - carPos[0], 2)*1.0 + pow(ballPos[1] - carPos[1], 2)*1.0 + pow(ballPos[2] - carPos[2], 2) * 1.0);
   
    //check the collision of the car and the ball.
    if (distance < ball_.radius() + car_.collision_radius()) {
        Vector3 normal = Vector3::Normalize(ballPos - carPos);
        ballPos = carPos + normal * (ball_.radius() + car_.collision_radius());
        Vector3 vRel = ballVel - carVel;
        Vector3 reflectVel = vRel-2*normal * (Vector3::Dot(normal, vRel));
        ballVel = carVel + reflectVel;
    }

   //update all the positions.
    ball_.set_velocity(ballVel);
    ball_.set_position(ballPos);
    car_.set_position(carPos);

    //Check if goal, if so, reset
    if ((-10 + ball_.radius()/2 < ballPos[0]) && (ballPos[0] < 10 - ball_.radius()/2) && (0 + ball_.radius()/2 < ballPos[1]) &&(ballPos[1] < 10 - ball_.radius()/2) && (ballPos[2] <= -48 + ball_.radius()|| ballPos[2] >= 48 - ball_.radius())) {
        ball_.launch_ball();
        car_.Reset();
    }

}


void CarSoccer::InitOpenGL() {
    // Set up the camera in a good position to see the entire field
    projMatrix_ = Matrix4::Perspective(60, aspect_ratio(), 1, 1000);
    modelMatrix_ = Matrix4::LookAt(Point3(0,60,70), Point3(0,0,10), Vector3(0,1,0));
 
    // Set a background color for the screen
    glClearColor(0.8,0.8,0.8, 1);
    
    // Load some image files we'll use
    fieldTex_.InitFromFile(Platform::FindFile("pitch.png", searchPath_));
    crowdTex_.InitFromFile(Platform::FindFile("crowd.png", searchPath_));
}


void CarSoccer::DrawUsingOpenGL() {
    // Draw the crowd as a fullscreen background image
    quickShapes_.DrawFullscreenTexture(Color(1,1,1), crowdTex_);
    
    // Draw the field with the field texture on it.
    Color col(16.0/255.0, 46.0/255.0, 9.0/255.0);
    Matrix4 M = Matrix4::Translation(Vector3(0,-0.201,0)) * Matrix4::Scale(Vector3(50, 1, 60));
    quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, col);
    M = Matrix4::Translation(Vector3(0,-0.2,0)) * Matrix4::Scale(Vector3(40, 1, 50));
    quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, Color(1,1,1), fieldTex_);
    
    // Draw the car
  /*  Color carcol(0.8, 0.2, 0.2);
    Matrix4 Mcar =
        Matrix4::Translation(car_.position() - Point3(0,0,0)) *
        Matrix4::Scale(car_.size()) *
        Matrix4::Scale(Vector3(0.5,0.5,0.5));
    quickShapes_.DrawCube(modelMatrix_ * Mcar, viewMatrix_, projMatrix_, carcol);
    */
    
    Color carcol(0.8, 0.2, 0.2);
    Matrix4 Mcar =
        Matrix4::Translation(car_.position() - Point3(0, 0, 0)) *
        Matrix4::Scale(car_.size()) *
        Matrix4::Scale(Vector3(0.5, 0.5, 0.5)) *
        m;
        quickShapes_.DrawCube(modelMatrix_ * Mcar, viewMatrix_, projMatrix_, carcol);


    
    // Draw the ball
    Color ballcol(1,1,1);
    Matrix4 Mball =
        Matrix4::Translation(ball_.position() - Point3(0,0,0)) *
        Matrix4::Scale(Vector3(ball_.radius(), ball_.radius(), ball_.radius()));
    quickShapes_.DrawSphere(modelMatrix_ * Mball, viewMatrix_, projMatrix_, ballcol);
    
    
    // Draw the ball's shadow -- this is a bit of a hack, scaling Y by zero
    // flattens the sphere into a pancake, which we then draw just a bit
    // above the ground plane.
    Color shadowcol(0.2,0.4,0.15);
    Matrix4 Mshadow =
        Matrix4::Translation(Vector3(ball_.position()[0], -0.1, ball_.position()[2])) *
        Matrix4::Scale(Vector3(ball_.radius(), 0, ball_.radius())) *
        Matrix4::RotationX(90);
    quickShapes_.DrawSphere(modelMatrix_ * Mshadow, viewMatrix_, projMatrix_, shadowcol);
    
    
    // You should add drawing the goals and the boundary of the playing area
    // using quickShapes_.DrawLines()
    
    //Draw lines around the boundary
   
    Color boundcol(22.0 / 255.0, 191.0 / 255.0, 247.0 / 255.0);
    std::vector<Point3> loop1;
    loop1.push_back(Point3(-40, 0, -50));
    loop1.push_back(Point3(40, 0, -50));
    loop1.push_back(Point3(40, 0, 50));
    loop1.push_back(Point3(-40, 0, 50));
    quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, boundcol, loop1, QuickShapes::LinesType::LINE_LOOP, 0.2);
    std::vector<Point3> loop2;
    loop2.push_back(Point3(-40, 35, -50));
    loop2.push_back(Point3(40, 35, -50));
    loop2.push_back(Point3(40, 35, 50));
    loop2.push_back(Point3(-40, 35, 50));
    quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, boundcol, loop2, QuickShapes::LinesType::LINE_LOOP, 0.2);
    std::vector<Point3> loop3;
    loop3.push_back(Point3(-40, 0, -50));
    loop3.push_back(Point3(40, 0, -50));
    loop3.push_back(Point3(40, 35, -50));
    loop3.push_back(Point3(-40, 35, -50));
    quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, boundcol, loop3, QuickShapes::LinesType::LINE_LOOP, 0.2);
    std::vector<Point3> loop4;
    loop4.push_back(Point3(-40, 0, 50));
    loop4.push_back(Point3(40, 0, 50));
    loop4.push_back(Point3(40, 35, 50));
    loop4.push_back(Point3(-40, 35, 50));
    quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, boundcol, loop4, QuickShapes::LinesType::LINE_LOOP, 0.2);

    //Draw a grid of lines for opposite goal.
    
    Color goalcol1(245.0 / 255.0, 135.0 / 255.0, 66.0 / 255.0);
    std::vector<Point3> goal1;
    for (int i = 0; i < 11; i++) {
            
            goal1.push_back(Point3(-10, 0 + i, -50));
            goal1.push_back(Point3(10, 0 + i, -50));
            quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, goalcol1, goal1, QuickShapes::LinesType::LINE_STRIP, 0.2);
        goal1.clear();
    }
    
    for (int i = 0; i < 21; i++) {
 
            goal1.push_back(Point3(-10 + i, 0, -50));
            goal1.push_back(Point3(-10 + i, 10, -50));
            quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, goalcol1, goal1, QuickShapes::LinesType::LINE_LOOP, 0.2);
 
        goal1.clear();
    }

    //Draw a grid of lines for our goal.
    Color goalcol2(123.0 / 255.0, 66.0 / 255.0, 245.0 / 255.0);
    std::vector<Point3> goal2;
    for (int i = 0; i < 11; i++) {

        goal2.push_back(Point3(-10, 0 + i, 50));
        goal2.push_back(Point3(10, 0 + i, 50));
        quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, goalcol2, goal2, QuickShapes::LinesType::LINE_STRIP, 0.2);
        goal2.clear();
    }

    
    for (int i = 0; i < 21; i++) {
 
            goal2.push_back(Point3(-10 + i, 0, 50));
            goal2.push_back(Point3(-10 + i, 10, 50));
            quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, goalcol2, goal2, QuickShapes::LinesType::LINE_STRIP, 0.2);
        goal2.clear();
    }
    
    
  
   
}
