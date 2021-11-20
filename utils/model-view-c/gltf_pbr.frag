/*
 * Physically Based Rendering Shader (WIP)
 */

const float M_PI        = 3.14159265358979323846;  /* pi */
#define MAX_LIGHTS      4

/* pbr_metallic_roughness */
uniform sampler2D baseColour_tex; /**< Base colour. */
uniform sampler2D metallic_tex; /**< Metallic texture. */
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 baseColour;
/* clearcoat */
uniform float clearcoat;
uniform float clearcoat_roughness;
uniform vec3 emissive;
uniform sampler2D emissive_tex; /**< Emission texture. */
/* misc */
uniform sampler2D occlusion_tex; /**< Ambient occlusion. */

in vec2 tex_coord0;
in vec3 position;
in vec3 normal;
out vec4 colour_out;

float pow5( float x ) {
   float x2 = x * x;
   return x2 * x2 * x;
}

vec3 pow5( vec3 x ) {
   vec3 x2 = x * x;
   return x2 * x2 * x;
}

float D_GGX( float roughness, float NoH )
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
vec3 F_Schlick(vec3 f0, vec3 f90, float VdotH)
{
   return f0 + (f90 - f0) * pow5(clamp(1.0 - VdotH, 0.0, 1.0));
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

vec3 BRDF_diffuse( vec3 diffuseColor, float roughness, float NoV, float NoL, float LoH )
{
   return diffuseColor * Fd_Burley( roughness, NoV, NoL, LoH );
}
#if 0
vec3 BRDF_lambertian( vec3 f0, vec3 f90, vec3 diffuseColor, float specularWeight, float VdotH )
{
   // see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
   return (1.0 - specularWeight * F_Schlick(f0, f90, VdotH)) * (diffuseColor / M_PI);
}
#endif

vec3 BRDF_specularGGX( vec3 f0, vec3 f90, float roughness, float VoH, float NoL, float NoV, float NoH )
{
   float D = D_GGX( roughness, NoH );
   float V = V_SmithGGXCorrelated( roughness, NoV, NoL );
   vec3 F  = F_Schlick( f0, f90, VoH );
   return (D * V) * F;
}

struct Material {
   vec3 albedo;         /**< Surface albedo. */
   float roughness;     /**< Surface roughness. */
   float metallic;      /**< Metallicness of the object. */
   vec3 f0;             /**< Fresnel value at 0 degrees. */
	vec3 f90;				/**< Fresnel value at 90 degrees. */
	vec3 c_diff;
   float roughness_cc;  /**< Clear coat roughness. */
   float clearCoat;     /**< Clear coat colour. */

	/* KHR_materials_specular */
	//float specularWeight; /**< product of specularFactor and specularTexture.a */
};

struct Light {
   vec3 position;
   float range;
   vec3 colour;
   float intensity;
};

uniform Light u_lights[ MAX_LIGHTS ];
uniform int u_nlights = 1;

vec3 light_intensity( Light L, float dist )
{
   float attenuation;
   if (L.range < 0.0)
      attenuation =  1.0 / pow(dist,2.0);
   else
      attenuation = max(min(1.0 - pow(dist / L.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
   return L.colour * L.intensity * attenuation;
}

#if 0
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
#endif

float clampedDot( vec3 x, vec3 y )
{
   return clamp(dot(x, y), 0.0, 1.0);
}

void main (void)
{
   /* Compute normal taking into account the bump map. */
   vec3 n = normal;
   //if (bm > 0.01)
   //   n += bm * texture(map_Bump, tex_coord0).xyz * 2.0 - 1.0;
   //norm = mix( norm, normal, 0.999 );
   n = normalize(n);

   /* Material values. */
   Material M;
   //M.albedo       = baseColour.rgb * texture(baseColour_tex, tex_coord0).rgb;
   M.albedo       = baseColour.rgb * texture(baseColour_tex, tex_coord0).rgb;
   M.roughness    = roughnessFactor * roughnessFactor; /* Convert from perceptual roughness. */
   M.metallic     = metallicFactor;
   M.f0           = mix( vec3(0.04), M.albedo, M.metallic );
   M.f90          = vec3(1.0);
	M.c_diff       = mix( M.albedo * (vec3(1.0) - M.f0), vec3(0), M.metallic);
   M.clearCoat    = clearcoat;
   M.roughness_cc = clearcoat_roughness;
	//M.specularWeight = 1.0;

   vec3 f_specular = vec3(0.0);
   vec3 f_diffuse  = vec3(0.0);
   vec3 f_emissive = vec3(0.0);
   vec3 f_clearcoat= vec3(0.0);

   /* Would have to do IBL lighting here. */
   //f_specular += getIBLRadianceGGX(n, v, materialInfo.perceptualRoughness, materialInfo.f0, materialInfo.specularWeight);
   //f_diffuse += getIBLRadianceLambertian(n, v, materialInfo.perceptualRoughness, materialInfo.c_diff, materialInfo.f0, materialInfo.specularWeight);
   //f_clearcoat += getIBLRadianceGGX(materialInfo.clearcoatNormal, v, materialInfo.clearcoatRoughness, materialInfo.clearcoatF0, 1.0);
   f_diffuse += 0.5*M.c_diff; /* Just use ambience for now. */

   /* Ambient occlusion. */
   float ao = texture(occlusion_tex, tex_coord0).r;
   f_diffuse *= ao;

   /* Point light for now. */
   const vec3 v = normalize( vec3(0.0, 0.0, 1.0) );
   for (int i=0; i<u_nlights; i++) {
      Light L = u_lights[i];

      vec3 pointToLight = L.position - position;
      vec3 l = normalize(pointToLight);
      vec3 h = normalize(l + v); /* Halfway vector. */
      float NoL = clampedDot(n, l);
      //float NoV = clampedDot(n, v);
      float NoV = clamp(dot(n,v), 1e-4, 1.0); /* Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886" */
      float NoH = clampedDot(n, h);
      float LoH = clampedDot(l, h);
      float VoH = clampedDot(v, h);

      /* Habemus light. */
      /* TODO this check will always be true if we do the NoV trick above. */
      //if (NoL > 0.0 || NoV > 0.0) {
         vec3 intensity = NoL * light_intensity( L, length(pointToLight) );

         //f_diffuse  += intensity * BRDF_lambertian( M.f0, M.f90, M.c_diff, M.specularWeight, VdotH );
         f_diffuse += intensity * BRDF_diffuse( M.c_diff, M.roughness, NoV, NoL, LoH );
         f_specular += intensity * BRDF_specularGGX( M.f0, M.f90, M.roughness, VoH, NoL, NoV, NoH );
      //}

   }

   //vec4 em = texture(emissive_tex, tex_coord0);
   //f_emissive = emissive * em.rgb * em.a;
   f_emissive = emissive * texture(emissive_tex, tex_coord0).rgb;

   colour_out = vec4( f_emissive + f_diffuse + f_specular, 1.0 );
   //colour_out = vec4( M.albedo, 1.0 );
   //colour_out = vec4( vec3(ao), 1.0 );
   //colour_out = vec4( f_emissive, 1.0 );
}
