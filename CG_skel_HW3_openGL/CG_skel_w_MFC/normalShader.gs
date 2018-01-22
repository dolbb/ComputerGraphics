#version 330
layout(triangles) in;

layout(line_strip, max_vertices=6) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalTransform;

const float len = 0.1;

in Vertex
{
  vec3 normal;
  vec4 color;
} vertex[];

out vec4 vertex_color;

void main()
{
  int i;
  for(i=0; i<gl_in.length(); i++)
  {
    vec4 startPoint = model * gl_in[i].gl_Position;
    vec3 nor = normalize(normalTransform * vertex[i].normal);
    vec4 endPoint = startPoint + vec4(nor,1.0) * len;
    
    gl_Position = projection * view * startPoint;
    vertex_color = vertex[i].color;
    EmitVertex();
    
    gl_Position = projection * view * endPoint;
    vertex_color = vertex[i].color;
    EmitVertex();
    
    EndPrimitive();
  }
}