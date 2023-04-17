#ifndef __ULIB_CHUGL_H__
#define __ULIB_CHUGL_H__

#include "chuck_dl.h"
#include <mutex>
#include <queue>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#inclu  de <GL/glut.h>
#endif

#include <iostream>

DLL_QUERY chugl_query( Chuck_DL_Query * QUERY );


// command IDs
enum ChuGLCommandID {
    color3,
    triangle,
    rect,
    ellipse,
    clear,
};

// command pattern
// for now just going to make this struct
// maybe eventually create abstract command class
class ChuGLCommand {
public:
    ChuGLCommand(ChuGLCommandID command_id, t_CKUINT size) {
        id = command_id; // maps to opengl instruction
        data_size = size;  
        data = malloc(data_size);
    }

    ~ChuGLCommand() {
        free(data);
    }

    void writeArg(size_t offset, void *newData, size_t size) {
        void * writePos = (char *) data + offset;
        memcpy(writePos, newData, size);
    }

    void * getArg(size_t offset) {
        return (void *) ((char *) data + offset);
    }

    ChuGLCommandID getID() {
        return id;
    }

private:
    ChuGLCommandID id; // make an enum?
    void *data;  // contains args for the graphics call
    t_CKUINT data_size;  // size of data buffer
};


// TODO make this a static class?
class ChuGL
{
public:
    ChuGL() {};
    ~ChuGL() {};

    // graphics api
    static void color3(float r, float g, float b) {
        // std::cout << "coloring";
        ChuGLCommand *command = new ChuGLCommand(ChuGLCommandID::color3, 3 * sizeof(float));
        
        size_t off = sizeof(float);
        command->writeArg(0, &r, sizeof(float));
        command->writeArg(off, &g, sizeof(float));
        command->writeArg(2*off, &b, sizeof(float));

        const std::lock_guard<std::mutex> lock(command_queue_mutex);
        command_queue.push(command);
    }

    static void rect(float x, float y, float width, float height) {
        // std::cout << "rect";
        ChuGLCommand *command = new ChuGLCommand(ChuGLCommandID::rect, 4 * sizeof(float));
        
        size_t off = sizeof(float);
        command->writeArg(0, &x, sizeof(float));
        command->writeArg(off, &y, sizeof(float));
        command->writeArg(2*off, &width, sizeof(float));
        command->writeArg(3*off, &height, sizeof(float));

        const std::lock_guard<std::mutex> lock(command_queue_mutex);
        command_queue.push(command);
    }

    static void clear() {
        ChuGLCommand *command = new ChuGLCommand(ChuGLCommandID::clear, 0);
        const std::lock_guard<std::mutex> lock(command_queue_mutex);
        command_queue.push(command);
    }

    static std::mutex& getLock() {
        return command_queue_mutex;
    }

    static std::queue<ChuGLCommand *>& getQueue() {
        return command_queue;
    }

    // executes all queued commands and frees them
    static void flushCommandQueue() {

        // std::cerr << "flushing" << std::flush;
        

        const std::lock_guard<std::mutex> lock(command_queue_mutex);

        while (!command_queue.empty()) {
            auto command = command_queue.front();
            command_queue.pop();

            auto id = command->getID();
            if (id == ChuGLCommandID::color3) {
                size_t off = sizeof(float);
                glColor4f(
                    * (float *) command->getArg(0), 
                    * (float *) command->getArg(off), 
                    * (float *) command->getArg(2*off), 
                    1.0
                );
            } else if (id == ChuGLCommandID::rect) {
                size_t off = sizeof(float);
                
                float x = * (float *) command->getArg(0);
                float y = * (float *) command->getArg(off);
                float width = * (float *) command->getArg(2*off);
                float height = * (float *) command->getArg(3*off);

                glBegin(GL_TRIANGLE_STRIP);
                glVertex3f(x, y, 0);
                glVertex3f(x+width, y, 0);
                glVertex3f(x, y+height, 0);
                glVertex3f(x+width, y+height, 0);
                glEnd();
            } else if (id == ChuGLCommandID::clear) {
                glClear(GL_COLOR_BUFFER_BIT);
            }

            // release the memory
            delete command;
        }

    }

    // void color4(float r, float g, float b, float a);
    // void ellipse(float x, float y, float width, float height);
private:
    static std::queue<ChuGLCommand *> command_queue;
    static std::mutex command_queue_mutex;  // lock on _staging buffers
};

#endif // _ULIB_CHUGL_H_