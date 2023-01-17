#include "Bezier1D.h"
#include "MeshConstructor.h"
#include <iostream>
#include <cstdio>

//parameters:
int points_per_segment = 20;
float curve_scale = 2;

/*
Bezier1D::Bezier1D(int segNum,int res,int mode, int viewport): 
    //printf("1\n");
    segmentsNum(segNum),
    resT(res),
    M(
        glm::mat4(
            -1, 3, -3, 1,
            3, -6, 3, 0,
            -3, 3, 0, 0,
            1, 0, 0, 0
        )
    ),
    segments(BuildSegments()), //build the segments
    Shape(new MeshConstructor(GetLine(), false), 1) //build the mesh constructor
{ 
    int x = 2;
}
*/
// TODO: diff between segNum and res
Bezier1D::Bezier1D(int segNum, int res, int mode, int viewport) : Shape(1)
{
    this->segmentsNum = segNum;   
    this->resT = res; 
    this->M = glm::mat4(
            -1, 3, -3, 1,
            3, -6, 3, 0,
            -3, 3, 0, 0,
            1, 0, 0, 0
        );
    this->segments = BuildSegments(); //build the segments
    this->setMesh(new MeshConstructor(GetLine(), false));

}

std::vector<glm::mat4> Bezier1D::BuildSegments()
{
    std::vector<glm::mat4> segments;
    //first segment
    glm::vec4 p0 = curve_scale * glm::vec4(0, 0, 0, 0);
    glm::vec4 p1 = curve_scale * glm::vec4(0, 1.5, 0, 0);
    glm::vec4 p2 = curve_scale * glm::vec4(0.5, 2, 0, 0);
    glm::vec4 p3 = curve_scale * glm::vec4(2, 2, 0, 0);
    glm::mat4 seg1(p0, p1, p2, p3);
    segments.push_back(seg1);
    
    // throw exption for resT = 2
    for (int i = 0; i < resT - 2; i++) {
        //ith segment
        p0 = segments.back()[3];
        p1 = p0 + curve_scale * glm::vec4(1, 0, 0, 0);
        p2 = p1 + curve_scale * glm::vec4(1, 0, 0, 0);
        p3 = p2 + curve_scale * glm::vec4(1, 0, 0, 0);
        glm::mat4 segI(p0, p1, p2, p3);
        segments.push_back(segI);
    }

    //last segment
    p0 = segments.back()[3];
    p1 = p0 + curve_scale * glm::vec4(1.5, 0, 0, 0);
    p2 = p1 + curve_scale * glm::vec4(0.5, -0.5, 0, 0);
    p3 = p2 + curve_scale * glm::vec4(0, -1.5, 0, 0);
    glm::mat4 segN(p0, p1, p2, p3);
    segments.push_back(segN);

    return segments;
}

IndexedModel Bezier1D::GetLine() const
{
    IndexedModel model;

    std::vector<LineVertex> axisVertices;
    for (int i = 0; i < segmentsNum; i++) {
        for (float t = 0; t < points_per_segment; t++) {
            glm::vec4 point = GetPointOnCurve(i, t/points_per_segment); //calculate le position of the point on the curve
            // printf("(%f, %f, %f),\n", point.x, point.y, point.z);
            LineVertex lv(glm::vec3(point.x, point.y, point.z), glm::vec3(1,1,1));
            axisVertices.push_back(lv);
            model.positions.push_back(*lv.GetPos()); //add position to the model
            model.colors.push_back(*lv.GetColor()); //add color to the model
            // if (i == 0 && t == 0) {
                model.indices.push_back(i*points_per_segment + t); //if it's the first point, add only it 
            // } else {
            //     model.indices.push_back(i*points_per_segment + t - 1); 
            //     model.indices.push_back(i*points_per_segment + t); //add the previous point and current point to indices (this makes sure all points are connected in the scene)  
            // }
        }
    }

    return model;
}

glm::vec4 Bezier1D::GetControlPoint(int segment, int indx) const
{
    if (segment < segmentsNum)
        return segments[segment][indx];
    return segments[segmentsNum - 1][3];
}

glm::vec4 Bezier1D::GetPointOnCurve(int segment, float t) const
{
    // t/=points_per_segment;
    glm::vec4 T(std::pow(t, 3), std::pow(t, 2), t, 1);
    glm::mat4 seg = segments[segment];
    glm::vec4 point = T * M * glm::transpose(seg);
    // printf("(%f, %f, %f)\n", point.x, point.y, point.z);
    return point;
}

glm::vec4 Bezier1D::GetVelocity(int segment, float t)
{
    glm::vec4 dT(3 * std::pow(t, 2), 2 * t, 1, 0);
    glm::vec4 vel = dT * M * glm::transpose(segments[segment]);
    return vel;
}

// void Bezier1D::SplitSegment(int segment, int t)
// {
// }

void Bezier1D::AddSegment(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments.back()[3];
    segments.push_back(glm::mat4(p0, p1, p2, p3));
}

void Bezier1D::ChangeSegment(int segIndx,glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments[segIndx-1][3];
    segments[segIndx] = glm::mat4(p0, p1, p2, p3);
}

float Bezier1D::MoveControlPoint(int segment, int indx, float dx,float dy,bool preserveC1)
{
    glm::vec4 delta(dx, dy, 0, 0);
    segments[segment][indx] += delta;
    if (preserveC1) {
        if (indx == 1 && segment != 0) {
            segments[segment - 1][indx + 1] -= delta;
        } else if (indx == 2 && segment != segmentsNum - 1) {
            segments[segment + 1][indx - 1] -= delta;
        }
    }

    this->setMesh(new MeshConstructor(GetLine(), false));
    // Draw(1,0,BACK,true,false);
	// glfwSwapBuffers(window);

    return 0; //not suppose to reach here
}

void Bezier1D::CurveUpdate(int pointIndx, float dx, float dy, bool preserveC1)
{
    //TODO: here the IndexModel will be updated, hopefully this will show in the scene, if not we need to find a way to make the scene change
}

void Bezier1D::ResetCurve(int segNum)
{

}

// int Bezier1D::GetSegmentsNum()
// {
//     return segmentsNum;
// }

// int Bezier1D::GetResT()
// {
//     return resT;
// }

Bezier1D::~Bezier1D(void)
{

}