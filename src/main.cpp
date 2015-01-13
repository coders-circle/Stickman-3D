#include <common.h>
#include <Renderer.h>

Renderer g_renderer;

// Point<N> defines a point with N-number of attributes
// Each attribute is a vec4
// So to store only color, the number of attributes is 1
typedef Point<1> ClrPoint;
void FragmentShader(ClrPoint& point)
{
    g_renderer.PutPixel(point.pos[0], point.pos[1], vec3(point.varying[0]));
}

ClrPoint pt1(200, 200);
ClrPoint pt2( 50, 200);
ClrPoint pt3(150, 400);

void Render()
{
    /*int pt1[] = { 200, 200 };
    int pt2[] = {  50, 200 };
    int pt3[] = { 150, 400 };*/
    //for (int i=0; i<500; ++i)
    g_renderer.DrawTriangle(&pt1, &pt2, &pt3, &FragmentShader);
}

int main()
{
    // Change the color attribute
    pt1.varying[0] = vec4(1.0f, 0.0f, 0.0f);
    pt2.varying[0] = vec4(0.0f, 1.0f, 0.0f);
    pt3.varying[0] = vec4(0.0f, 0.0f, 1.0f);
    g_renderer.Initialize("Test Renderer", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
