#version 110

varying vec4 V;
varying vec3 N;

void main()
{
	// light position in eye space (relative to camera)
	const vec3 light = vec3(0.0, 0.0, 0.0);

	// calculate lighting vectors
	vec3 L = normalize( light - V.xyz );   
	vec3 E = normalize( -V.xyz ); 
	vec3 R = normalize( -reflect(L,N) );   

	// diffuse term
	vec4 diffuse = gl_Color;
	diffuse *= max( dot(N,L), 0.0 );
	diffuse = clamp( diffuse, 0.0, 1.0 );     

	// specular term
	const float shininess = 25.0;

	vec4 specular = vec4(0.5, 0.5, 0.5, 1.0); 
	specular *= pow( max(dot(R,E),0.0), shininess );
	specular = clamp( specular, 0.0, 1.0 ); 

	// final color 
	gl_FragColor = diffuse + specular;
}