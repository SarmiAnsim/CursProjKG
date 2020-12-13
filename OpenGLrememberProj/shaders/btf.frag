 varying vec2 texCoord;
 varying vec3 Normal;
 uniform sampler2D iTexture0;
 uniform sampler2D iTexture1;

 varying vec3 Position;
 vec3 Ia = vec3(0.2,0.2,0.2);
 vec3 Id = vec3(0.7,0.7,0.7);
 vec3 Is = vec3(1.0,1.0,1.0);
 vec3 ma = vec3(0.6,0.6,0.6);
 vec3 md = vec3(1,1,1);
 vec3 ms = vec3(1,1,1);
 
 float  alpha = 10.0;
 
 varying vec3 iLightPos_mv;
 varying vec3 iCamPos_mv;

void easyButter(void)
{
	Normal = vec3(texture2D(iTexture1, texCoord)).rgb;
 	Normal = Normal * 2.0 - 1.0;
 	Normal = (gl_NormalMatrix * Normal).xyz;
    	Normal = normalize(Normal);
	
 	vec3 color_amb = Ia*ma;
	
	vec3 light_vector = normalize(iLightPos_mv - Position);
	vec3 color_dif = Id*md*dot( light_vector, Normal);
	
	vec3 cam_vector = normalize(iCamPos_mv - Position);
	vec3 reflect_vector = reflect(-light_vector,Normal);
	float cosRC = max(0.0,dot(cam_vector,reflect_vector));	
	vec3 col_spec = Is*ms*pow(cosRC,300);
	
	gl_FragColor = vec4(color_amb + color_dif + col_spec,1);
    
    // Без полупрозрачности
    //vec4 color = vec4(texture2D(iTexture0,texCoord).rgb,1) * vec4(color_amb + color_dif + col_spec,1.0 * (texture2D(iTexture0,texCoord).rgb == 1 ? 0:1));
    
    // Полупрозрачные крылья
    vec4 color = vec4(texture2D(iTexture0,texCoord).rgb,1) * vec4(color_amb + color_dif + col_spec,1.0 * (1-texture2D(iTexture0,texCoord).rgb));
    
    if(texture2D(iTexture0,texCoord).rgb == 1)
        discard;

    gl_FragColor = color;
}

void main(void)
 {
 	easyButter();
 }