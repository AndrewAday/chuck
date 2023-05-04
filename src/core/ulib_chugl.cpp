#include "ulib_chugl.h"

CK_DLL_CTOR(chugl_ctor);
CK_DLL_DTOR(chugl_dtor);
CK_DLL_SFUN(chugl_color3);
CK_DLL_MFUN(chugl_rect);
CK_DLL_SFUN(chugl_clear);

// CK_DLL_MFUN(chugl_color4);
// CK_DLL_MFUN(chugl_triangle);
// CK_DLL_MFUN(chugl_ellipse);
// CK_DLL_MFUN(chugl_line);

static t_CKUINT chugl_data_offset = 0;

DLL_QUERY chugl_query( Chuck_DL_Query * QUERY )
{
    // get environment reference
    Chuck_Env * env = QUERY->env();

    // set name
    QUERY->setname( QUERY, "ChuGL" );

    // function
    Chuck_DL_Func * func = NULL;
    // documentation string
    std::string doc;

    // begin class
    QUERY->begin_class( QUERY, "ChuGL", "Object" );
    QUERY->doc_class( QUERY, "Class for graphics rendering" );

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    chugl_data_offset = QUERY->add_mvar(QUERY, "int", "@chugl_data", false);

    // register the constructor
    QUERY->add_ctor(QUERY, chugl_ctor);
    // register the destructor
    QUERY->add_dtor(QUERY, chugl_dtor);

    // Graphics API
    // these copied from Spencer's chugl
    QUERY->add_sfun(QUERY, chugl_color3, "void", "color");
    QUERY->add_arg(QUERY, "float", "r");
    QUERY->add_arg(QUERY, "float", "g");
    QUERY->add_arg(QUERY, "float", "b");
    QUERY->doc_func(QUERY, "Set the current drawing color to the specified RGB color. ");

    QUERY->add_mfun(QUERY, chugl_rect, "void", "rect");
    QUERY->add_arg(QUERY, "float", "x");
    QUERY->add_arg(QUERY, "float", "y");
    QUERY->add_arg(QUERY, "float", "width"); 
    QUERY->add_arg(QUERY, "float", "height");
    QUERY->doc_func(QUERY, "Draw a rectangle with bottom-left corner at (x,y) and specified width and height. The current transform matrix and color properties will be used when drawing the rectangle. ");

    QUERY->add_sfun(QUERY, chugl_clear, "void", "clear");
    QUERY->doc_func(QUERY, "Clear the screen.");

    /*
    QUERY->add_mfun(QUERY, chugl_color4, "void", "color");
    QUERY->add_arg(QUERY, "float", "r");
    QUERY->add_arg(QUERY, "float", "g");
    QUERY->add_arg(QUERY, "float", "b");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set the current drawing color to the specified RGBA color. ");
    
    
    QUERY->add_mfun(QUERY, chugl_triangle, "void", "triangle");
    QUERY->add_arg(QUERY, "float", "ax"); QUERY->add_arg(QUERY, "float", "ay");
    QUERY->add_arg(QUERY, "float", "bx"); QUERY->add_arg(QUERY, "float", "by");
    QUERY->add_arg(QUERY, "float", "cx"); QUERY->add_arg(QUERY, "float", "cy");
    QUERY->doc_func(QUERY, "Draw a triangle described by the specified vertices. The current transform matrix and color properties will be used when drawing the triangle. ");
    
    QUERY->add_mfun(QUERY, chugl_ellipse, "void", "ellipse");
    QUERY->add_arg(QUERY, "float", "x");     QUERY->add_arg(QUERY, "float", "y");
    QUERY->add_arg(QUERY, "float", "width"); QUERY->add_arg(QUERY, "float", "height");
    QUERY->doc_func(QUERY, "Draw an ellipse bottom-left corner at (x,y) and specified width and height. The current transform matrix and color properties will be used when drawing the ellipse. ");
    
    QUERY->add_mfun(QUERY, chugl_line, "void", "line");
    QUERY->add_arg(QUERY, "float", "x1"); QUERY->add_arg(QUERY, "float", "y1");
    QUERY->add_arg(QUERY, "float", "x2"); QUERY->add_arg(QUERY, "float", "y2");
    QUERY->doc_func(QUERY, "Draw a line between (x1,y1) and (x2,y2). The current transform matrix and color properties will be used when drawing the line. ");
    */

    QUERY->end_class( QUERY );

    return TRUE;

//error:
//    return FALSE;
}

// chugl implementation

// static initialization
std::queue<ChuGLCommand *> ChuGL::command_queue;
std::mutex ChuGL::command_queue_mutex;  // lock on _staging buffers

// std::atomic_flag ChuGL::clear_flag = ATOMIC_FLAG_INIT;
int ChuGL::clear_flag = 0;

std::unordered_set<ull> ChuGL::command_uid_set;

unsigned long long ChuGLCommand::uid_counter = 0;

CK_DLL_CTOR(chugl_ctor)
{
    OBJ_MEMBER_INT(SELF, chugl_data_offset) = (t_CKINT) new ChuGL;
}

CK_DLL_DTOR(chugl_dtor)
{
    ChuGL * chugl = (ChuGL *) OBJ_MEMBER_INT(SELF, chugl_data_offset);
    SAFE_DELETE(chugl);
    OBJ_MEMBER_INT(SELF, chugl_data_offset) = 0;
}

CK_DLL_SFUN(chugl_color3)
{
    // ChuGL *chugl = (ChuGL*) OBJ_MEMBER_INT(SELF, chugl_data_offset);
    
    t_CKFLOAT r = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT g = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT b = GET_NEXT_FLOAT(ARGS);

    ChuGL::color3(r, g, b);

    // chugl->color3(r, g, b);
}

CK_DLL_MFUN(chugl_rect) 
{
    ChuGL * chugl = (ChuGL *) OBJ_MEMBER_INT(SELF, chugl_data_offset);

    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT width = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT height = GET_NEXT_FLOAT(ARGS);

    chugl->rect(x, y, width, height);
}

CK_DLL_SFUN(chugl_clear)
{
    // ChuGL::clear();
    ChuGL::set_atomic_clear();
}