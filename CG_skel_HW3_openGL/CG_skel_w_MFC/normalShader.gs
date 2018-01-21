#version 330
layout(triangles) in;

layout(line_strip, max_vertices=6) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalTransform;

in Vertex
{
  vec4 normal;
  vec4 color;
} vertex[];

out vec4 vertex_color;

void main()
{
  int i;
  for(i=0; i<gl_in.length(); i++)
  {
    vec3 startPoint = model * gl_in[i].gl_Position.xyz;
    vec3 nor = normalTransform * vertex[i].normal.xyz;
    vec3 endPoint = normalize(startPoint + nor);
    
    gl_Position = projection * view * vec4(startPoint, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    gl_Position = projection * view * vec4(endPoint, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    EndPrimitive();
  }
}