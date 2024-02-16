#ifndef MAX_LIGHTS
#  define MAX_LIGHTS 3
#endif

/*
 * Physically Based Rendering Shader (WIP)
 */
const float M_PI        = 3.14159265358979323846;  /* pi */

/* pbr_metallic_roughness */
uniform sampler2D baseColour_tex; /**< Base colour. */
uniform bool baseColour_texcoord;
uniform sampler2D metallic_tex; /**< Metallic texture. */
uniform bool metallic_texcoord;
#ifdef HAS_NORMAL
uniform bool u_has_normal; /**< Whether or not has a normal map. */
uniform sampler2D normal_tex; /**< Normal map. */
uniform bool normal_texcoord;
uniform float normal_scale;
#endif /* HAS_NORMAL */
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 baseColour;
/* Sheen. */
uniform vec3 sheenTint;
uniform float sheen;

/* Clearcoat */
uniform float clearcoat;
uniform float clearcoat_roughness;
uniform vec3 emissive;
uniform sampler2D emissive_tex; /**< Emission texture. */
uniform bool emissive_texcoord;
/* misc */
#ifdef HAS_AO
uniform sampler2D occlusion_tex; /**< Ambient occlusion. */
uniform bool occlusion_texcoord;
#endif /* HAS_AO */
uniform int u_blend;
uniform vec3 u_ambient; /**< Ambient lighting. */
uniform float u_waxiness;

/**
 * @brief Lighting information.
 */
struct Light {
   bool sun;         /**< Whether or not a sun. */
   vec3 position;    /**< Position or orientation if sun. */
   vec3 colour;      /**< Colour to use. */
   float intensity;  /**< Strength of the light. */
};
uniform int u_nlights = 1;
uniform Light u_lights[ MAX_LIGHTS ];
uniform sampler2D shadowmap_tex[ MAX_LIGHTS ];

/* Input/Output */
in vec2 tex_coord0;
in vec2 tex_coord1;
in vec3 position;
in vec3 shadow[MAX_LIGHTS];
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

float rgb2lum( vec3 color ) {
   return dot( color, vec3( 0.2989, 0.5870, 0.1140 ) );
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

/* Disney Principled BRDF sheen. */
vec3 BRDF_sheen( float LoH, vec3 baseColour, vec3 sheenTint, float sheen )
 {
    float FH = pow5( clamp(1.0-LoH, 0.0, 1.0) ); /* Schlick Fresnel. */
    // Approximate luminance
    float Cdlum = rgb2lum(baseColour);
    vec3 Ctint = (Cdlum > 0.0) ? baseColour / Cdlum : vec3(1.0);
    vec3 Csheen = mix( vec3(1.0), Ctint, sheenTint );
    return FH * sheen * Csheen;
}

/* Disney Principled BRDF subsurface. */
vec3 BRDF_subsurface( float NoL, float NoV, float LoH, vec3 c_diff, float roughness  )
{
   float FL = pow5( clamp(1.0-NoL, 0.0, 1.0) );
   float FV = pow5( clamp(1.0-NoV, 0.0, 1.0) );
   float Fss90 = LoH * LoH * roughness;
   float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
   float ss = 1.25 * (Fss * (1. / (NoL + NoV) - 0.5) + 0.5);
   return (1.0/M_PI) * ss * c_diff;
}

struct Material {
   vec4 albedo;         /**< Surface albedo. */
   float perceptualRoughness;
   float roughness;     /**< Surface roughness. */
   float metallic;      /**< Metallicness of the object. */
   vec3 f0;             /**< Fresnel value at 0 degrees. */
   vec3 f90;            /**< Fresnel value at 90 degrees. */
   vec3 c_diff;
   vec3 sheenTint;
   float sheen;
   float clearcoat;     /**< Clear coat colour. */
   float clearcoat_roughness;/**< Clear coat roughness. */

   /* KHR_materials_specular */
   //float specularWeight; /**< product of specularFactor and specularTexture.a */
};

vec3 light_intensity( Light L, float dist )
{
#if 0
   float attenuation;
   if (L.range < 0.0)
      attenuation =  1.0 / pow(dist,2.0);
   else
      attenuation = max(min(1.0 - pow(dist / L.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
#endif
   float attenuation =  1.0 / pow(dist,2.0);
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

/* Taken from https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders */
vec3 get_normal (void)
{
#if HAS_NORMAL
   if (u_has_normal==false)
      return normalize(normal);

   vec2 coords = (normal_texcoord ? tex_coord1 : tex_coord0);

   vec2 uv    = coords;
   vec2 uv_dx = dFdx(uv);
   vec2 uv_dy = dFdy(uv);

   vec3 t_ = (uv_dy.t * dFdx(position) - uv_dx.t * dFdy(position)) /
             (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);

   vec3 ng = normalize(normal);
   vec3 t = normalize(t_ - ng * dot(ng, t_));
   vec3 b = cross(ng, t);

   /* Negate for back facing surfaces. */
   if (gl_FrontFacing == false) {
      t *= -1.0;
      b *= -1.0;
      ng *= -1.0;
   }

   vec3 n = texture(normal_tex, coords).rgb * vec3(normal_scale, normal_scale, 1.0) * 2.0 - vec3(1.0);
   return normalize( mat3(t, b, ng) * n);
#else /* HAS_NORMAL */
   return normalize(normal);
#endif /* HAS_NORMAL */
}

float linstep( float minval, float maxval, float val )
{
   return clamp( (val-minval) / (maxval-minval), 0., 1. );
}

float shadow_map( sampler2D tex, vec3 pos )
{
   /* Variance Shadow Mapping. */
   /* Compute Moments (gpu gems 3 chapter 8). */
   float m1 = texture( tex, pos.xy ).r;
   float m1dx = dFdx(m1);
   float m1dy = dFdy(m1);
   float m2 = m1*m1 + 0.25*(m1dx*m1dx + m1dy*m1dy);

   /* Chebyshev Uppe Bound. */
   float p = float(pos.z <= m1);
   float var = m2 - m1*m1;
   const float minvar = 0.0001;
   var = max( var, minvar );
   float d = pos.z - m1;
   float p_max = var / (var + d*d);
   /* Trick to reduce light-bleeding. */
   p_max = linstep( 0.1, 1.0, p_max );
   return max( p, p_max );

   /* Regular Shadow Mapping. */
   //float f_shadow = (l_depth_m1 + 0.05 > shadow.z) ? 1.0 : 0.0;
   //return f_shadow;
}

void main (void)
{
   vec3 n = get_normal();

   /* Material values. */
   Material M;
   //M.albedo    = baseColour.rgb * texture(baseColour_tex, tex_coord0).rgb;
   M.albedo    = baseColour * texture(baseColour_tex, (baseColour_texcoord ? tex_coord1 : tex_coord0));
   vec4 metallicroughness = texture(metallic_tex, (metallic_texcoord ? tex_coord1: tex_coord0));
   M.perceptualRoughness = roughnessFactor * metallicroughness.g;
   M.roughness = M.perceptualRoughness * M.perceptualRoughness; /* Convert from perceptual roughness. */
   M.metallic  = metallicFactor * metallicroughness.b;
   M.f0        = mix( vec3(0.04), M.albedo.rgb, M.metallic );
   M.f90       = vec3(1.0);
   M.c_diff    = mix( M.albedo.rgb, vec3(0.), M.metallic );
   M.sheenTint = sheenTint;
   M.sheen     = sheen;
   M.clearcoat = clearcoat;
   M.clearcoat_roughness = clearcoat_roughness * clearcoat_roughness;
   //M.specularWeight = 1.0;

   vec3 f_specular   = vec3(0.0);
   vec3 f_diffuse    = vec3(0.0);
   //vec3 f_subsurface = vec3(0.0);
   vec3 f_sheen      = vec3(0.0);
   vec3 f_emissive   = vec3(0.0);
   vec3 f_clearcoat  = vec3(0.0);

   /* Would have to do IBL lighting here. */
   //f_specular += getIBLRadianceGGX(n, v, materialInfo.perceptualRoughness, materialInfo.f0, materialInfo.specularWeight);
   //f_diffuse += getIBLRadianceLambertian(n, v, materialInfo.perceptualRoughness, materialInfo.c_diff, materialInfo.f0, materialInfo.specularWeight);
   //f_clearcoat += getIBLRadianceGGX(materialInfo.clearcoatNormal, v, materialInfo.clearcoatRoughness, materialInfo.clearcoatF0, 1.0);
   //f_diffuse += 0.5 * M.c_diff; /* Just use ambience for now. */

   /* Slight diffuse ambient lighting. */
   f_diffuse += u_ambient * M.c_diff;/* * (1.0 / M_PI); premultiplied */

   /* Ambient occlusion. */
#ifdef HAS_AO
   float ao = texture(occlusion_tex, (occlusion_texcoord ? tex_coord1 : tex_coord0)).r;
   f_diffuse *= ao;
#endif /* HAS_AO */

   /* Variance Shadow Mapping. */
   float f_shadow[MAX_LIGHTS];
   for (int i=0; i<u_nlights; i++)
      f_shadow[i] = 0.0;
#if MAX_LIGHTS > 0
   if (u_nlights>0)
      f_shadow[0] = shadow_map( shadowmap_tex[0], shadow[0] );
#endif /* MAX_LIGHTS > 0 */
#if MAX_LIGHTS > 1
   if (u_nlights>1)
      f_shadow[1] = shadow_map( shadowmap_tex[1], shadow[1] );
#endif /* MAX_LIGHTS > 1 */
#if MAX_LIGHTS > 2
   if (u_nlights>2)
      f_shadow[2] = shadow_map( shadowmap_tex[2], shadow[2] );
#endif /* MAX_LIGHTS > 2 */

   /* Point light for now. */
   const vec3 v = normalize( vec3(0.0, 0.0, -1.0) ); /* Fixed view vector. */
   float NoV = clamp(dot(n,v), 1e-4, 1.0); /* Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886" */
   for (int i=0; i<u_nlights; i++) {
      Light L = u_lights[i];
      vec3 l;
      vec3 intensity;

      if (L.sun) {
         l = normalize(L.position);
         intensity = L.colour * L.intensity;
      }
      else {
         vec3 pointToLight = L.position - position;
         l = normalize(pointToLight);
         intensity = light_intensity( L, length(pointToLight) );
      }

      vec3 h = normalize(l + v); /* Halfway vector. */
      float NoL = clampedDot(n, l);
      //float NoV = clampedDot(n, v);
      float NoH = clampedDot(n, h);
      float LoH = clampedDot(l, h);
      float VoH = clampedDot(v, h);

      /* Habemus light. */
      /* TODO this check will always be true if we do the NoV trick above. */
      //if (NoL > 0.0 || NoV > 0.0) {
         vec3 NoLintensity = NoL * intensity * f_shadow[i];

#if 0
         //f_diffuse  += intensity * BRDF_lambertian( M.f0, M.f90, M.c_diff, M.specularWeight, VdotH );
         float NoL_diffuse;
         if (u_waxiness > 0.0)
            NoL_diffuse = u_waxiness + (1.0-u_waxiness) * NoL;
         else
            NoL_diffuse = NoL;
         f_diffuse  += NoLintensity * BRDF_diffuse( M.c_diff, M.roughness, NoV, NoL_diffuse, LoH );
#endif
         f_diffuse  += NoLintensity * BRDF_diffuse( M.c_diff, M.roughness, NoV, NoL, LoH );
         if (M.sheen > 0.0)
            f_sheen  += BRDF_sheen( LoH, M.c_diff, M.sheenTint, M.sheen );
         //f_subsurface  += NoLintensity * BRDF_subsurface( NoL, NoV, LoH, M.c_diff, M.roughness );
         f_specular += NoLintensity * BRDF_specularGGX( M.f0, M.f90, M.roughness, VoH, NoL, NoV, NoH );

         /* Clear coat lobe. */
         f_clearcoat += intensity * BRDF_specularGGX( M.f0, M.f90, M.clearcoat_roughness, VoH, NoL, NoV, NoH );
      //}
   }

   /* Do emissive. */
   f_emissive = emissive * texture(emissive_tex, (emissive_texcoord ? tex_coord1 : tex_coord0) ).rgb;

   /* Combine diffuse, emissive, and specular.. */
   float alpha = (u_blend==1) ? M.albedo.a : 1.0;
   //colour_out = vec4( f_emissive + mix(f_diffuse,f_subsurface,sheen) + f_sheen + f_specular, alpha );
   colour_out = vec4( f_emissive + f_diffuse + f_sheen + f_specular, alpha );

   /* Apply clearcoat. */
   vec3 clearcoatFresnel = F_Schlick( M.f0, M.f90, NoV );
   f_clearcoat *= M.clearcoat;
   colour_out.rgb = colour_out.rgb * (1.0 - M.clearcoat * clearcoatFresnel) + f_clearcoat;

   //colour_out = vec4( M.albedo.rgb, 1.0 );
   //colour_out = vec4( vec3(M.metallic), 1.0 );
   //colour_out = vec4( vec3(M.perceptualRoughness), 1.0 );
   //colour_out = vec4( vec3(ao), 1.0 );
   //colour_out = vec4( f_emissive, 1.0 );
   //colour_out = vec4( (n*0.5+0.5), 1.0 );
   //colour_out = vec4( vec3(sheen), 1.0 );
   //colour_out = vec4( vec3(sheenTint), 1.0 );
}
