#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#ifdef USE_GLEW
    #include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif

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

    morph::softmats::TriangleMesh* initialMesh;

    std::vector<GLfloat> g_uv_buffer_data;
    std::vector<GLfloat> g_vertex_buffer_data;
    std::vector<GLfloat> g_colour_buffer_data;

    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint colbuffer;
    GLuint progID;

    sceneObject(GLuint progID, morph::softmats::TriangleMesh* mesh){

        initialMesh = mesh;
        this->progID = progID;

        std::vector<float> pvalues;
        std::vector<float> tvalues;
        std::vector<float> cvalues;

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

            cvalues.push_back(0); //r
            cvalues.push_back(0); //g
            cvalues.push_back(0); //b
            cvalues.push_back(0); //r
            cvalues.push_back(0); //g
            cvalues.push_back(0); //b
            cvalues.push_back(0); //r
            cvalues.push_back(0); //g
            cvalues.push_back(0); //b

        }
        g_vertex_buffer_data = pvalues;
        g_uv_buffer_data = tvalues;
        g_colour_buffer_data = cvalues;
    }

    ~sceneObject(void){

    }

    void updatePositions(morph::softmats::TriangleMesh* mesh){
        std::vector<morph::softmats::Face *>& faces = mesh->getFaces();
        int k=0;
        for( morph::softmats::Face* f : faces ){
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[0]->x(i); k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[1]->x(i); k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[2]->x(i); k++; }
        }
    }

    void updatePositions(float scale, std::vector<float> pos){

        std::vector<morph::softmats::Face *>& faces = initialMesh->getFaces();

        int k=0;
        for( morph::softmats::Face* f : faces ){
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[0]->x(i)*scale+pos[i]; k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[1]->x(i)*scale+pos[i]; k++; }
            for(int i=0; i<3; i++) { g_vertex_buffer_data[k] = f->points[2]->x(i)*scale+pos[i]; k++; }
        }

    }


    void updatePositions(float scale, std::vector<float> pos, std::vector<float> rot){

        std::vector<morph::softmats::Face *>& faces = initialMesh->getFaces();

        // see: https://www.mathworks.com/matlabcentral/answers/500030-rotate-a-3d-data-cloud-to-align-with-one-axis

        // components of reference axis through the original point cloud that is to be re-aligned (default +y)
        float rx = 0.0;
        float ry = 1.0;
        float rz = 0.0;

        // components of axis to which the reference axis should be aligned
        float vx = rot[0];
        float vy = rot[1];
        float vz = rot[2];

        // normalize it
        float normv = 1./pow(vx*vx+vy*vy+vz*vz,0.5);
        vx *= normv;
        vy *= normv;
        vz *= normv;

        // angle between reference axis r and velocity axis
        float ang = acos(vx*rx+vy*ry+vz*rz);

        // cross product of reference and velocity axes
        float cx = vy*rz-vz*ry;
        float cy = vz*rx-vx*rz;
        float cz = vx*ry-vy*rx;

        // normalize it
        float normc = 1./pow(cx*cx+cy*cy+cz*cz,0.5);
        cx *= normc;
        cy *= normc;
        cz *= normc;


        /*
        arma::mat R (3,3,arma::fill::eye);
        arma::mat M1 = {{0,-cz,cy},{cz,0,-cx},{-cy,cx,0}};
        arma::mat M2 = {{cx*cx,cx*cy,cx*cz},{cx*cy,cy*cy,cy*cz},{cx*cz,cy*cz,cz*cz}};
        arma::mat rotMat = R*cos(ang)+sin(ang)*M1+(1-cos(ang))*M2;
        arma::mat rotMat2 = {{ca+mca*cx*cx, -cz*sa+mca*cx*cy, cy*sa+mca*cx*cz},
                            {cz*sa+mca*cx*cy, ca+mca*cy*cy, -cx*sa+mca*cy*cz},
                            {-cy*sa+mca*cx*cz, cx*sa+mca*cy*cz, ca+mca*cz*cz}};
        */

        float ca = cos(ang);
        float sa = sin(ang);
        float mca = 1-ca;

        // rotation matrix
        float rot00 = ca+mca*cx*cx;
        float rot01 = -cz*sa+mca*cx*cy;
        float rot02 = cy*sa+mca*cx*cz;
        float rot10 = cz*sa+mca*cx*cy;
        float rot11 = ca+mca*cy*cy;
        float rot12 = -cx*sa+mca*cy*cz;
        float rot20 = -cy*sa+mca*cx*cz;
        float rot21 = cx*sa+mca*cy*cz;
        float rot22 = ca+mca*cz*cz;

        float rotx, roty, rotz;
        int k=0;

        // Rotate triangle vertices
        for( morph::softmats::Face* f : faces ){

            // First triangle vertex
            rotx = rot00*f->points[0]->x(0) + rot01*f->points[0]->x(1) + rot02*f->points[0]->x(2);
            roty = rot10*f->points[0]->x(0) + rot11*f->points[0]->x(1) + rot12*f->points[0]->x(2);
            rotz = rot20*f->points[0]->x(0) + rot21*f->points[0]->x(1) + rot22*f->points[0]->x(2);
            g_vertex_buffer_data[k+0] = rotx*scale+pos[0];
            g_vertex_buffer_data[k+1] = roty*scale+pos[1];
            g_vertex_buffer_data[k+2] = rotz*scale+pos[2];
            // Second triangle vertex
            rotx = rot00*f->points[1]->x(0) + rot01*f->points[1]->x(1) + rot02*f->points[1]->x(2);
            roty = rot10*f->points[1]->x(0) + rot11*f->points[1]->x(1) + rot12*f->points[1]->x(2);
            rotz = rot20*f->points[1]->x(0) + rot21*f->points[1]->x(1) + rot22*f->points[1]->x(2);
            g_vertex_buffer_data[k+3] = rotx*scale+pos[0];
            g_vertex_buffer_data[k+4] = roty*scale+pos[1];
            g_vertex_buffer_data[k+5] = rotz*scale+pos[2];
            // Third triangle vertex
            rotx = rot00*f->points[2]->x(0) + rot01*f->points[2]->x(1) + rot02*f->points[2]->x(2);
            roty = rot10*f->points[2]->x(0) + rot11*f->points[2]->x(1) + rot12*f->points[2]->x(2);
            rotz = rot20*f->points[2]->x(0) + rot21*f->points[2]->x(1) + rot22*f->points[2]->x(2);
            g_vertex_buffer_data[k+6] = rotx*scale+pos[0];
            g_vertex_buffer_data[k+7] = roty*scale+pos[1];
            g_vertex_buffer_data[k+8] = rotz*scale+pos[2];

            k+=9;
        }

    }


    void render(void){

        glUniform1i( glGetUniformLocation(progID, "type" ) , 0 ); // render texture

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
        glDisableVertexAttribArray(2);

        glDeleteBuffers(1, &vertexbuffer);
        glDeleteBuffers(1, &colbuffer);
        glDeleteBuffers(1, &uvbuffer);
        
    }

    void render(float red, float green, float blue){

        glUniform1i( glGetUniformLocation(progID, "type" ) , 1 );   // render colour

        for(int i=0;i<g_colour_buffer_data.size();i++){
            switch(i%3){
                case(0):{
                    g_colour_buffer_data[i] = red;
                } break;
                case(1):{
                    g_colour_buffer_data[i] = green;
                } break;
                case(2):{
                    g_colour_buffer_data[i] = blue;
                } break;
            }
        }


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
        glGenBuffers(1, &colbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*g_colour_buffer_data.size(), &g_colour_buffer_data[0], GL_STATIC_DRAW);

        // Vertex attribute buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

        // Colour attribute buffer
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, colbuffer);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

        // Draw the triangles
        glDrawArrays(GL_TRIANGLES, 0, g_vertex_buffer_data.size()/3);
        glDisableVertexAttribArray(0);
        //glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glDeleteBuffers(1, &vertexbuffer);
        glDeleteBuffers(1, &colbuffer);
        glDeleteBuffers(1, &uvbuffer);
    }
};




class Scene {

public:

    GLuint programID, MatrixID, VertexArrayID;
    morph::TransformMatrix<float> Projection, Model, MVP;
    camera* Cam;
    GLuint vertexbuffer, uvbuffer;
    GLFWwindow* window;
    GLint windowWidth, windowHeight;
    std::vector<sceneObject> mySceneObjects;
    std::vector<sceneObject> scenePrimitives;
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

#ifdef USE_GLEW
        if( glewInit() == GLEW_OK ){ std::cout << "Glew initialized\n"; }
        else { std::cout << "Glew initialized failed \n"; exit(EXIT_FAILURE); }
        glewExperimental = true;
#endif

        glfwSwapInterval( 1 );

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

    void addScenePrimitive(std::string pathToMeshFile){
        morph::softmats::ObjMeshProvider mp( pathToMeshFile.c_str() );
        morph::softmats::TriangleMesh* mesh = mp.buildMesh();
        scenePrimitives.push_back(sceneObject(programID,mesh));
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

    /*
    void update(void){
        preDraw();
        draw();
        postDraw();
    }
    */


    void update(void){
        preDraw();

        draw();

        glUniform1i( glGetUniformLocation(programID, "type" ) , 1 );

        for(int i=0;i<mySceneObjects.size();i++){

            // get vertices of the scene object
            std::vector<morph::softmats::Point *>& V = mySceneObjects[i].initialMesh->getVertices();

            // update origin of primitive to mark each vertex
            std::vector<float> pos(3,0);
            std::vector<float> rot(3,0);
            int q=0;

            for( morph::softmats::Point* p : V ){

                pos[0] = p->x[0];
                pos[1] = p->x[1];
                pos[2] = p->x[2];

                rot[0] = V[q]->v(0);
                rot[1] = V[q]->v(1);
                rot[2] = V[q]->v(2);

                scenePrimitives[0].updatePositions(0.05, pos, rot);
                scenePrimitives[0].render(0,1,0);

                q++;
            }
        }


        //draw();
        postDraw();
    }

    void setClearColour(float R, float G, float B, float A){
        glClearColor(R,G,B,A);
    }

    virtual void buildScene(void){
        addTexture("template.bmp");
        morph::softmats::ObjMeshProvider mp("sphere.obj");
        morph::softmats::TriangleMesh* mesh = mp.buildMesh();
        addSceneObject(sceneObject(programID,mesh));
        pairObjectTexture(0,0);
    }

    ~Scene(void){
        glDeleteProgram(programID);
        glDeleteVertexArrays(1, &VertexArrayID);
        glfwTerminate();
    }

};
