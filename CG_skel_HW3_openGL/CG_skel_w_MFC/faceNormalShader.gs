#version 150

layout(triangles) in;
layout(line_strip, max_vertices=2) out;

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
  vec3 v0 = gl_in[0].gl_Position.xyz;
  vec3 v1 = gl_in[1].gl_Position.xyz;
  vec3 v2 = gl_in[2].gl_Position.xyz;
  
  vec3 e0 = v0 - v1;
  vec3 e1 = v2 - v1;
  
  vec3 fNormal = normalTransform * normalize(cross(e0, e1));
  vec3 fCenter = model * (v0+v1+v2) / 3.0;
  
  vec3 endPoint = fCenter + fNormal;

  gl_Position = projection * view * vec4(fCenter, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  
  gl_Position = projection * view * vec4(endPoint, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  EndPrimitive();
}