/*
 * Physically Based Rendering Shader (WIP)
 */

const float M_PI        = 3.14159265358979323846;  /* pi */

/* Textures. */
uniform sampler2D map_Kd;  /* Diffuse map. */
uniform sampler2D map_Ks;  /* Specular map. */
uniform sampler2D map_Ke;  /* Emission map. */
uniform sampler2D map_Bump;/* Bump map. */

/* Phong model parameters. */
uniform float Ns; /* Specular shininess. */
uniform vec3 Ka;  /* Ambient colour. */
uniform vec3 Kd;  /* Diffuse colour. */
uniform vec3 Ks;  /* Specular colour. */
uniform vec3 Ke;  /* Emissive colour. */
uniform float Ni; /* Optical density. */
uniform float d;  /* Dissolve (opacity). */

uniform float bm; /* Bump mapping parameter. */

in vec2 tex_coord;
in vec3 pos;
in vec3 normal;
out vec4 colour_out;

float pow5(float x) {
   float x2 = x * x;
   return x2 * x2 * x;
}

float D_GGX( float roughness, float NoH, const vec3 h )
{
	/* Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces" */
	float oneMinusNoHSquared = 1.0 - NoH * NoH;

	float a = NoH * roughness;
	float k = roughness / (oneMinusNoHSquared + a * a);
	float d = k * k * (1.0 / M_PI);
	return clamp(d,0.0,1.0);
}

float V_SmithGGXCorrelated( float roughness, float NoV, float NoL )
{
	/* Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs" */
	float a2 = roughness * roughness;
	// TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
	float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
	float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
	float v = 0.5 / (lambdaV + lambdaL);
	// a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
	// a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
	// clamp to the maximum value representable in mediump
	return clamp(v,0.0,1.0);
}

float V_Kelemen( float LoH )
{
   /* Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling" */
   return clamp(0.25 / (LoH * LoH), 0.0, 1.0);
}

vec3 F_Schlick( const vec3 f0, float f90, float VoH )
{
	/* Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering" */
	return f0 + (f90 - f0) * pow5(1.0 - VoH);
}
vec3 F_Schlick( const vec3 f0, float VoH )
{
	float f = pow5(1.0 - VoH);
	return f + f0 * (1.0 - f);
}
float F_Schlick( float f0, float f90, float VoH )
{
	return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float Fd_Lambert (void)
{
	return 1.0 / M_PI;
}

float Fd_Burley( float roughness, float NoV, float NoL, float LoH )
{
	/* Burley 2012, "Physically-Based Shading at Disney" */
	float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
	float lightScatter = F_Schlick(1.0, f90, NoL);
	float viewScatter  = F_Schlick(1.0, f90, NoV);
	return lightScatter * viewScatter * (1.0 / M_PI);
}

struct Material {
   vec3 albedo;         /**< Surface albedo. */
   float roughness;     /**< Surface roughness. */
   vec3 F0;             /**< Fresnel value at 0 degrees. */
   float roughness_cc;  /**< Clear coat roughness. */
   float clearCoat;     /**< Clear coat colour. */
};

struct Light {
   vec3 position;
   vec3 colour;
};

vec3 shade( Material mat, vec3 v, vec3 n, vec3 l, float NoL )
{
   vec3 h   = normalize(l+v); /* Halfway vector. */

   /* Compute helpers. */
   float NoV = max(1e-4,dot(n,v)); /* Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886" */
   float NoH = max(0.0,dot(n,h));
   float LoH = max(0.0,dot(l,h));
   float VoH = max(0.0,dot(v,h));

   /* Specular Lobe. */
   float D = D_GGX( mat.roughness, NoH, h );
   float V = V_SmithGGXCorrelated( mat.roughness, NoV, NoL );
   vec3  F = F_Schlick( mat.F0, VoH );
   vec3 Fr = (D * V) * F;

   /* Diffuse Lobe. */
   vec3 Fd = mat.albedo * Fd_Burley( mat.roughness, NoV, NoL, LoH );
   //vec3 Fd = Td * Fd_Lambert();

	/* The energy compensation term is used to counteract the darkening effect
	 * at high roughness */
	//vec3 colour = Fd + Fr * pixel.energyCompensation;
   vec3 colour = Fd + Fr;

   /* Clear coat Lobe. */
   float Dcc = D_GGX( mat.roughness_cc, NoH, h );
   float Vcc = V_Kelemen( LoH );
   float Fcc = F_Schlick(0.04, 1.0, LoH) * mat.clearCoat; // fix IOR to 1.5
   float Fc  = (Dcc * Vcc) * Fcc;
   colour *= (1.0-Fc) * NoL;
   colour += Fc * NoL;

   return colour;
}

void main(void) {
   /* Compute normal taking into account the bump map. */
   vec3 n = normal;
   if (bm > 0.01)
      n += bm * texture(map_Bump, tex_coord).xyz * 2.0 - 1.0;
   //norm = mix( norm, normal, 0.999 );
   n = normalize(n);

   /* Material values. */
   float roughness =  0.1;
   /* Set up textures. */
   vec3 Td = texture(map_Kd, tex_coord).rgb;
   vec3 Ta = Td; // Assume ambient is the same as dispersion
   vec3 Ts = texture(map_Ks, tex_coord).rgb;
   vec3 Te = texture(map_Ke, tex_coord).rgb;

   Material mat;
   mat.albedo        = Td;
   mat.roughness     = 0.05;
   mat.F0            = vec3(0.56); /* Iron. */
   mat.clearCoat     = 1.0;
   mat.roughness_cc  = 0.01;

   /* Get the crew ready. */
   /* Point light for now. */
   const vec3 lp  = vec3(3.0, 3.0, 3.0);
   const vec3 v   = normalize( vec3(0.0, 1.0, 1.0) );
   vec3 p   = pos;
   vec3 l   = normalize(lp-p);
   float NoL = max(0.0,dot(n,l));

   vec3 colour = shade( mat, v, n, l, NoL );

   colour_out = vec4(colour * NoL, 1.0) * 2.0;
}
