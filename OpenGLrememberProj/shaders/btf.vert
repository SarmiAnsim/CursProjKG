 varying vec2 texCoord;
 varying vec3 Normal;
 varying vec3 Position;
 uniform mat4 iModelViewMatrix;
 
 uniform vec3 iLightPos; 
 uniform vec3 iCamPos; 
 
 varying vec3 iLightPos_mv;
 varying vec3 iCamPos_mv;
 
 
void main(void)
{

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix*vec4(gl_Vertex.xyz,1.0);
    Position = (gl_ModelViewMatrix*gl_Vertex).xyz;
    texCoord = gl_MultiTexCoord0.xy;
    Normal = (gl_NormalMatrix * gl_Normal).xyz;
    Normal = normalize(Normal);
    
    iLightPos_mv = (iModelViewMatrix*vec4(iLightPos,1.0)).xyz;    
    iCamPos_mv = (iModelViewMatrix*vec4(iCamPos,1.0)).xyz;
}