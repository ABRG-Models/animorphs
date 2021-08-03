#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "morph/TransformMatrix.h"
#include "../util/meshutil.h"
#include "../core/face.h"
#include "../util/openglutils.h"


class camera {

    private:
        morph::TransformMatrix<float> orientation, direction;

    public:
    float speed;
    float mouse_sensitivity;
    float horzAng;
    float vertAng;
    morph::Vector<float, 3> pos;
    GLFWwindow* window;
    double timeNow, timeBefore;

    camera(GLFWwindow* window){
        speed = 4.0f;
        mouse_sensitivity = 0.01;
        horzAng = 0.;
        vertAng = 0.;
        pos = {0.,0.,5.};
        this->window = window;
        timeNow = glfwGetTime();
        timeBefore = glfwGetTime();

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwSetCursorPos(window, winWidth/2,winHeight/2);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED | GLFW_CURSOR_HIDDEN);
    }

    morph::TransformMatrix<float> update(void){

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        timeBefore = timeNow;
        timeNow = glfwGetTime();
        double timeDiff = (timeNow-timeBefore);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        horzAng = mouse_sensitivity*(float)(mouseX-winWidth/2);
        vertAng = mouse_sensitivity*(float)(mouseY-winHeight/2);

        orientation.setToIdentity();
        morph::Quaternion<float> q;
        q.rotate (1,0,0, -vertAng);
        q.rotate (0,1,0, -horzAng);
        orientation.rotate(q);

        if(glfwGetKey(window, GLFW_KEY_UP)){
            direction.setToIdentity();
            direction.rotate(q);
            morph::Vector<float, 3> off = ((direction.invert()) * morph::Vector<float, 4>({0,0,-1,1})).less_one_dim();
            pos += off * speed * timeDiff;
        } else if (glfwGetKey(window, GLFW_KEY_DOWN)){
            direction.setToIdentity();
            direction.rotate(q);
            morph::Vector<float, 3> off = ((direction.invert()) * morph::Vector<float, 4>({0,0,-1,1})).less_one_dim();
            pos -= off * speed * timeDiff;
        }

        if(glfwGetKey(window, GLFW_KEY_RIGHT)){
            direction.setToIdentity();
            direction.rotate(q);
            morph::Vector<float, 3> off = ((direction.invert()) * morph::Vector<float, 4>({1,0,0,1})).less_one_dim();
            pos += off * speed * timeDiff;
        } else if (glfwGetKey(window, GLFW_KEY_LEFT)){
            direction.setToIdentity();
            direction.rotate(q);
            morph::Vector<float, 3> off = ((direction.invert()) * morph::Vector<float, 4>({1,0,0,1})).less_one_dim();
            pos -= off * speed * timeDiff;
        }

        if(glfwGetKey(window, 'Z')){
            pos += morph::Vector<float, 3>({0,1,0}) * speed * timeDiff;
        } else if (glfwGetKey(window, 'X')){
            pos += morph::Vector<float, 3>({0,-1,0}) * speed * timeDiff;
        }

        morph::TransformMatrix<float> trans;
        trans.translate(-pos);
        return orientation * trans;
    }
};



class sceneObject {

public:

    std::vector<GLfloat> g_uv_buffer_data;
    std::vector<GLfloat> g_vertex_buffer_data;

    GLuint vertexbuffer;
    GLuint uvbuffer;

    sceneObject(morph::softmats::TriangleMesh* mesh){

        std::vector<float> pvalues;
        std::vector<float> tvalues;

        std::vector<morph::softmats::Face *>& faces = mesh->getFaces();

        for( morph::softmats::Face* f : faces ){

            for(int i=0; i<3; i++) { pvalues.push_back(f->points[0]->x(i)); }
            for(int i=0; i<3; i++) { pvalues.push_back(f->points[1]->x(i)); }
            for(int i=0; i<3; i++) { pvalues.push_back(f->points[2]->x(i)); }

            tvalues.push_back(0);   // lower-left u
            tvalues.push_back(0);   // lower-left v
            tvalues.push_back(0);   // upper-left u
            tvalues.push_back(1);   // upper-left v
            tvalues.push_back(1);   // upper-right u
            tvalues.push_back(1);   // upper-right v
        }
        g_vertex_buffer_data = pvalues;
        g_uv_buffer_data = tvalues;
    }

    ~sceneObject(void){ }

    void updatePositions(morph::softmats::TriangleMesh* mesh){
        std::vector<morph::softmats::Face *>& faces = mesh->getFaces();
        int k=0;
        for( morph::softmats::Face* f : faces ){
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[0]->x(i); k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[1]->x(i); k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[2]->x(i); k++; }
        }
    }

    void render(void){

        /*
        // glVertexAttribPointer variables (for reference):
            // Must match the layout in the shader!
            // size
            // type
            // normalized?
            // stride
            // array buffer offset
        */

        //Generate and bind the vertex buffer, then send to OpenGL
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*g_vertex_buffer_data.size(), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

        //Generate and bind the texture (uv) buffer, then send to OpenGL
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*g_uv_buffer_data.size(), &g_uv_buffer_data[0], GL_STATIC_DRAW);

        // Vertex attribute buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

        // Texture (uv) attribute buffer
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

        // Draw the triangles
        glDrawArrays(GL_TRIANGLES, 0, g_vertex_buffer_data.size()/3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
};




class Scene {

public:

    GLuint programID, MatrixID;
    morph::TransformMatrix<float> Projection, Model, MVP;
    camera* Cam;
    GLuint vertexbuffer, uvbuffer;
    GLFWwindow* window;
    GLint windowWidth, windowHeight;
    std::vector<sceneObject> mySceneObjects;
    std::vector<GLuint> textures;
    std::vector<int> ObjTexPair;

    bool running;

    Scene (std::string pathToVertexShader, std::string pathToFragmentShader){

        running = true;

        if( glfwInit()) { std::cout << "GLFW initialized \n"; }
        else {  std::cout << "GLFW initialization failed \n"; exit(EXIT_FAILURE); }

        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 ); // OpenGL 4.1 is max supported on Mac
        #ifdef __OSX__
        glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #endif
        window = glfwCreateWindow( 600, 600, "model", NULL, NULL );

        glfwMakeContextCurrent( window );
        if (window) { std::cout << "Window created\n"; }
        else { std::cout << "Window creation failed \n"; exit(EXIT_FAILURE);}

        if( glewInit() == GLEW_OK ){ std::cout << "Glew initialized\n"; }
        else { std::cout << "Glew initialized failed \n"; exit(EXIT_FAILURE); }

        glfwSwapInterval( 1 );
        glewExperimental = true;

        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearColor(0.87, 0.94, 1.0, 1.0 );

        programID = morph::softmats::OpenglUtils::createShaderProgram(pathToVertexShader.c_str(),pathToFragmentShader.c_str());

        MatrixID = glGetUniformLocation(programID, "MVP");

        Cam = new camera(window);

        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        Projection.perspective( 60.0f, (double)windowWidth/(double)windowHeight, 0.1f, 1000.0f );
        Model.setToIdentity();
        MVP = Projection * Model;

    }

    void addSceneObject(sceneObject S){
        mySceneObjects.push_back(S);
        ObjTexPair.push_back(0);
    }

    void pairObjectTexture(int obj, int tex){
        ObjTexPair[obj]=tex;
    }

    void addTexture(std::string fname){
        textures.push_back(morph::softmats::OpenglUtils::loadTextureImage(fname.c_str()));
    }

    void preDraw(void){
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        Model = Cam->update();
        MVP = Projection * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP.mat.data());
    }

    void draw(void){

        for(int i=0;i<mySceneObjects.size();i++){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[ObjTexPair[i]]);
            mySceneObjects[i].render();
        }
    }

    void postDraw(){
        glfwSwapBuffers(window);
        glfwPollEvents();
        running = !glfwGetKey(window, GLFW_KEY_ESCAPE );
    }

    void update(void){
        preDraw();
        draw();
        postDraw();
    }

    void setClearColour(float R, float G, float B, float A){
        glClearColor(R,G,B,A);
    }

    virtual void buildScene(void){
        addTexture("template.bmp");
        morph::softmats::ObjMeshProvider mp("sphere.obj");
        morph::softmats::TriangleMesh* mesh = mp.buildMesh();
        addSceneObject(sceneObject(mesh));
        pairObjectTexture(0,0);
    }

    ~Scene(void){
        glfwTerminate();
    }

};
