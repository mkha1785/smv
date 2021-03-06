#include "options.h"

#ifdef pp_GPU
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "glew.h"

#include "smokeviewvars.h"

GLhandleARB p_smoke, p_3dslice, p_zonesmoke, p_volsmoke;

#define LINK_BAD 0
#define LINK_GOOD 1

/* ------------------ PrintInfoLog ------------------------ */

void PrintInfoLog(GLhandleARB obj){
  int infologLength = 0;
  int charsWritten = 0;
  char *infoLog;

  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
  if(infologLength > 0){
    NewMemory((void **)&infoLog, infologLength);
    glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
    PRINTF("%s\n", infoLog);
    FREEMEMORY(infoLog);
  }
}

/* ------------------ SetZoneSmokeShaders ------------------------ */

int SetZoneSmokeShaders(){
  GLhandleARB vert_shader, frag_shader;
  GLint error_code;

  const GLchar *FragmentShaderSource[] = {
    "uniform int zonedir,zoneinside;"
    "uniform float xyzmaxdiff,zlay,odl,odu;"
    "uniform vec3 eyepos,boxmin, boxmax;"
    "varying vec3 fragpos;"

    "void main(){"
    "  float L,opacity,alpha,alpha_min,alpha_zlay;"
    "  float factor_U, factor_L, grey;"

    "  vec3 dalphamin,dalphamax;"
    "  L=distance(fragpos,eyepos)*xyzmaxdiff;"
    "  alpha_min=1000000.0;"
    "  dalphamin=-(boxmin-fragpos)/(eyepos-fragpos);"
    "  dalphamax=-(boxmax-fragpos)/(eyepos-fragpos);"
    "  alpha_zlay = -(zlay-fragpos.z)/(eyepos.z-fragpos.z);"
    "  factor_U=1.0;"
    "  factor_L=0.0;"
    "  if(zoneinside==0){"
    "    if(zonedir!=-1&&dalphamin.x>0.0&&dalphamin.x<alpha_min)alpha_min=dalphamin.x;"
    "    if(zonedir!=1 &&dalphamax.x>0.0&&dalphamax.x<alpha_min)alpha_min=dalphamax.x;"
    "    if(zonedir!=-2&&dalphamin.y>0.0&&dalphamin.y<alpha_min)alpha_min=dalphamin.y;"
    "    if(zonedir!=2 &&dalphamax.y>0.0&&dalphamax.y<alpha_min)alpha_min=dalphamax.y;"
    "    if(zonedir!=-3&&dalphamin.z>0.0&&dalphamin.z<alpha_min)alpha_min=dalphamin.z;"
    "    if(zonedir!=3 &&dalphamax.z>0.0&&dalphamax.z<alpha_min)alpha_min=dalphamax.z;"
    "    if(eyepos.z>zlay&&fragpos.z>zlay){"
    "      if(alpha_zlay>0.0&&alpha_zlay<alpha_min){"
    "        factor_U=alpha_zlay/odu;"
    "        factor_L=(alpha_min-alpha_zlay)/odl;"
    "      }"
    "      else{"
    "        factor_U=alpha_min/odu;"
    "        factor_L=0.0;"
    "      }"
    "    }"
    "    else if(eyepos.z>zlay&&fragpos.z<=zlay){"
    "      factor_U=0.0;"
    "      factor_L=alpha_min/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z>zlay){"
    "      factor_U=alpha_min/odu;"
    "      factor_L=0.0;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z<=zlay){"
    "      if(alpha_zlay>0.0&&alpha_zlay<alpha_min){"
    "        factor_U=(alpha_min-alpha_zlay)/odu;"
    "        factor_L=alpha_zlay/odl;"
    "      }"
    "      else{"
    "        factor_U=0.0;"
    "        factor_L=alpha_min/odl;"
    "      }"
    "    }"
    "  }" // end inside=0
    "  if(zoneinside==1){"
    "    if(eyepos.z>zlay&&fragpos.z>zlay){"
    "      factor_U=1.0/odu;"
    "      factor_L=0.0;"
    "    }"
    "    else if(eyepos.z>zlay&&fragpos.z<=zlay){"
    "      factor_U=(1.0+alpha_zlay)/odu;"
    "      factor_L=-alpha_zlay/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z>zlay){"
    "      factor_U=-alpha_zlay/odu;"
    "      factor_L=(1.0+alpha_zlay)/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z<=zlay){"
    "      factor_U=0.0;"
    "      factor_L=1.0/odl;"
    "    }"
    "  }" // end inside=1
    "  grey=0.0;"
    "  opacity = 1.0-exp(-(factor_L+factor_U)*L);"
    "  gl_FragColor = vec4(grey,grey,grey,opacity);"
    "}" // end of main
  };

  const GLchar *VertexShaderSource[]={
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position = ftransform();"
    "}"
  };

  vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShaderARB(vert_shader);

  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShaderARB(frag_shader);

#ifdef _DEBUG
  PrintInfoLog(vert_shader);
  PrintInfoLog(frag_shader);
#endif

  p_zonesmoke = glCreateProgramObjectARB();
  glAttachObjectARB(p_zonesmoke,vert_shader);
  glAttachObjectARB(p_zonesmoke,frag_shader);

  glLinkProgram(p_zonesmoke);
  glGetObjectParameterivARB(p_zonesmoke,GL_OBJECT_LINK_STATUS_ARB,&error_code);
#ifdef _DEBUG
  PRINTF("  Zone Smoke shader completion code:");
  switch(error_code){
  case GL_INVALID_VALUE:
    PRINTF(" INVALID VALUE\n");
    break;
  case GL_INVALID_OPERATION:
    PRINTF(" INVALID OPERATION\n");
    break;
  case GL_INVALID_ENUM:
    PRINTF(" INVALID ENUM\n");
    break;
  case LINK_BAD:
    PRINTF(" Link failed\n");
    break;
  case LINK_GOOD:
    PRINTF(" Link succeeded\n");
    break;
  default:
    PRINTF(" unknown error\n");
    break;
  }
  PrintInfoLog(p_zonesmoke);
#endif
  GPUzone_zoneinside = glGetUniformLocation(p_zonesmoke,"zoneinside");
  GPUzone_zonedir = glGetUniformLocation(p_zonesmoke,"zonedir");
  GPUzone_eyepos = glGetUniformLocation(p_zonesmoke,"eyepos");
  GPUzone_xyzmaxdiff = glGetUniformLocation(p_zonesmoke,"xyzmaxdiff");
  GPUzone_boxmin = glGetUniformLocation(p_zonesmoke,"boxmin");
  GPUzone_boxmax = glGetUniformLocation(p_zonesmoke,"boxmax");
  GPUzone_zlay = glGetUniformLocation(p_zonesmoke,"zlay");
  GPUzone_odl = glGetUniformLocation(p_zonesmoke,"odl");
  GPUzone_odu = glGetUniformLocation(p_zonesmoke,"odu");

  if(error_code!=1)return error_code;
  return error_code;

}

/* ------------------ Set3DSliceShaders ------------------------ */

int Set3DSliceShaders(void){
  GLhandleARB vert_shader, frag_shader;
  GLint error_code;

  const GLchar *FragmentShaderSource[]={
   "  uniform sampler1D colormap;"
   "  uniform sampler3D val_texture;"
   "  uniform float val_min,val_max;"
   "  uniform float transparent_level;"
   "  varying vec3 fragpos;"
   "  uniform vec3 boxmin,boxmax;"
   "void main(){"
   "  vec3 color_val,position;"
   "  float val,colorindex;"

   "  position = (fragpos-boxmin)/(boxmax-boxmin);"
   "  val = texture3D(val_texture,position);"
   "  colorindex = (val-val_min)/(val_max-val_min);"
   "  colorindex = clamp(colorindex,0.0,1.0);"
   "  color_val = texture1D(colormap,colorindex).rgb;"
   "  gl_FragColor = vec4(color_val.rgb,transparent_level);"
   "}"
  };

  const GLchar *VertexShaderSource[]={
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position=ftransform();"
    "}"
  };

  vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShaderARB(vert_shader);

  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShaderARB(frag_shader);

#ifdef _DEBUG
  PrintInfoLog(vert_shader);
  PrintInfoLog(frag_shader);
#endif

  p_3dslice = glCreateProgramObjectARB();
  glAttachObjectARB(p_3dslice,vert_shader);
  glAttachObjectARB(p_3dslice,frag_shader);

  glLinkProgram(p_3dslice);
  glGetObjectParameterivARB(p_3dslice,GL_OBJECT_LINK_STATUS_ARB,&error_code);
#ifdef _DEBUG
  PRINTF("  3D Slice shader completion code:");
  switch(error_code){
  case GL_INVALID_VALUE:
    PRINTF(" INVALID VALUE\n");
    break;
  case GL_INVALID_OPERATION:
    PRINTF(" INVALID OPERATION\n");
    break;
  case GL_INVALID_ENUM:
    PRINTF(" INVALID ENUM\n");
    break;
  case LINK_BAD:
    PRINTF(" Link failed\n");
    break;
  case LINK_GOOD:
    PRINTF(" Link succeeded\n");
    break;
  default:
    PRINTF(" unknown error\n");
    break;
  }
  PrintInfoLog(p_3dslice);
#endif

  GPU3dslice_valtexture = glGetUniformLocation(p_3dslice,"valtexture");
  GPU3dslice_colormap = glGetUniformLocation(p_3dslice,"colormap");
  GPU3dslice_val_min = glGetUniformLocation(p_3dslice,"val_min");
  GPU3dslice_val_max = glGetUniformLocation(p_3dslice,"val_max");
  GPU3dslice_boxmin = glGetUniformLocation(p_3dslice,"boxmin");
  GPU3dslice_boxmax = glGetUniformLocation(p_3dslice,"boxmax");
  GPU3dslice_transparent_level = glGetUniformLocation(p_3dslice,"transparent_level");

  if(error_code!=1)return error_code;
  return error_code;
}

/* ------------------ SetVolSmokeShaders ------------------------ */

int SetVolSmokeShaders(){
  GLhandleARB vert_shader, frag_shader;
  GLint error_code;

  const GLchar *FragmentShaderSource[] = {
    "uniform sampler1D smokecolormap;"
#ifdef pp_GPUDEPTH
    "uniform sampler2D depthtexture;"
    "uniform vec2 screensize;"
    "uniform vec2 nearfar;"
#endif
    "uniform sampler3D soot_density_texture,fire_texture,light_texture,blockage_texture;"
    "uniform vec3 eyepos,boxmin,boxmax,dcell3,light_color,light_position;"
    "varying vec3 fragpos;"
    "uniform float xyzmaxdiff,dcell,fire_opacity_factor,gpu_vol_factor;"
    "uniform float temperature_min,temperature_cutoff,temperature_max;"
    "uniform float voltemp_factor, voltemp_offset;"
    "uniform float mass_extinct, light_intensity, scatter_param;"
    "uniform int inside,havefire,volbw,slicetype,block_volsmoke,use_light;"
    "uniform int drawsides[7];"
    "uniform int scatter_type,light_type,vol_adaptive;"

#ifdef pp_GPUDEPTH
    // http://en.wikipedia.org/wiki/Depth_buffer#Mathematics
        "float LinearizeDepth(vec2 uv){"
        "  float near, far, z;"
        "  near=nearfar.x;"
        "  far=nearfar.y;"
        "  z = texture2D(depthtexture, uv).x;"
        "  return (near*far)/(far+z*(near-far));"
        "}"
    #endif

    "float color2bw(vec3 color){"
    " return 0.299*color.r+0.587*color.g+0.114*color.b;"
    "}"
    "void main(){"
    #ifdef pp_GPUDEPTH
    //  "  vec2 uv = gl_TexCoord[4].xy;"
      "  vec2 uv = gl_FragCoord.st/screensize.xy;"
  #endif
    "  vec3 dalphamin,dalphamax,fragmaxpos,position,position2,color_val,color_total,block_pos,block_pos2;"
    "  vec3 uvec, vvec;"
    "  float d;"
    "  float soot_val,block_val,block_val2;"
    "  float alpha_min,factor,factor0,dfactor,dfactor0,pathdist;"
    "  float colorindex,last_tempval,tempval,gray;"
    "  float taui, alphai;"
    "  float taun, alphan;"
    "  float light_fraction, light_factor, scatter_fraction;"
    "  float dstep;"
    "  float cos_angle,fourpi;"
    "  int i,n_iter;"
    "  int side,in_fire;"
    "  int in_block;"
    "  float opacity_factor;"
    "  float maxcolor;"

    "  alpha_min=1000000.0;"
    "  dalphamin=-(boxmin-fragpos)/(eyepos-fragpos);"
    "  dalphamax=-(boxmax-fragpos)/(eyepos-fragpos);"
    "  side=0;"
    "  if(inside==0){"
    "    if(drawsides[-1+3]==0&&dalphamin.x>0.0&&dalphamin.x<alpha_min){"
    "      alpha_min=dalphamin.x;"
    "      side=-1;"
    "    }"
    "    if(drawsides[ 1+3]==0&&dalphamax.x>0.0&&dalphamax.x<alpha_min){"
    "      alpha_min=dalphamax.x;"
    "      side=1;"
    "    }"
    "    if(drawsides[-2+3]==0&&dalphamin.y>0.0&&dalphamin.y<alpha_min){"
    "      alpha_min=dalphamin.y;"
    "      side=-2;"
    "    }"
    "    if(drawsides[ 2+3]==0&&dalphamax.y>0.0&&dalphamax.y<alpha_min){"
    "      alpha_min=dalphamax.y;"
    "      side=2;"
    "    }"
    "    if(drawsides[-3+3]==0&&dalphamin.z>0.0&&dalphamin.z<alpha_min){"
    "      alpha_min=dalphamin.z;"
    "      side=-3;"
    "    }"
    "    if(drawsides[ 3+3]==0&&dalphamax.z>0.0&&dalphamax.z<alpha_min){"
    "      alpha_min=dalphamax.z;"
    "      side=3;"
    "    }"
    "  }" // end inside=0
    "  if(inside==1){"
    "  }" // end inside=1
    "  fragmaxpos = mix(fragpos,eyepos,-alpha_min);"
#ifdef pp_GPUDEPTH
    "  d = LinearizeDepth(uv);"
    "  pathdist = d-distance(fragpos,eyepos);"
#else
    "  pathdist = distance(fragpos,fragmaxpos);"
#endif
    "  n_iter = int(gpu_vol_factor*pathdist/dcell+0.5);"
    "  if(n_iter<1)n_iter=1;"
    "  taun=1.0;"
    "  alphan=0.0;"
    "  in_fire=0;"
    "  color_total=vec3(0.0,0.0,0.0);"

    "  dfactor0 = 1.0/float(n_iter);"
    "  dfactor = dfactor0;"
    "  dstep = pathdist*xyzmaxdiff/float(n_iter);"

    "  factor0=0.5*dfactor;"
    "  factor=factor0;"

    "  last_tempval=0.0;"
    "  tempval=0.0;"
    "  position=vec3(0.0,0.0,0.0);"
    "  position2=position;"
    "  while(factor<1.0){"
    "    in_block=0;"
    "    position = (mix(fragpos,fragmaxpos,factor)-boxmin)/(boxmax-boxmin);"
    "    if(slicetype!=1){"
    //            boxmin+dcell3      position2     boxmax
    // boxmin                       position       boxmax
    // (position2-boxmin-dcell3)/(boxmax-boxmin-dcell3) = (position-boxmin)/(boxmax-boxmin)
    //  solve for position2
    "      position2=position+dcell3*(boxmax-position)/(boxmax-boxmin);"
    "    }"
    "    block_val=1.0;"
#ifndef pp_GPUDEPTH
    "    if(block_volsmoke==0){"
    "      block_val=1.0;"
    "      block_val2=1.0;"
    "    }"
    "    else{"
    "      block_pos = position;"
    "      block_pos2 = (mix(fragpos,fragmaxpos,factor+dfactor)-boxmin)/(boxmax-boxmin);"
    "      block_val = texture3D(blockage_texture,block_pos);"
    "      block_val2 = texture3D(blockage_texture,block_pos2);"
    "    }"
#endif
    "    if(slicetype==1){"
    "      soot_val = texture3D(soot_density_texture,position);"
    "    }"
    "    else{"
    "      soot_val = texture3D(soot_density_texture,position2);"
    "    }"
    "    if(block_val<0.5)soot_val=0.0;"
    "    opacity_factor = 1.0;"
    "    if(havefire==1){"
    "      if(slicetype==1){"
    "        tempval = texture3D(fire_texture,position);"
    "      }"
    "      else{"
    "        tempval = texture3D(fire_texture,position2);"
    "      }"
    "      if(tempval>voltemp_factor){"
    "        opacity_factor = (273.0+tempval)/(273.0+voltemp_factor);"
    "        opacity_factor = opacity_factor*opacity_factor*opacity_factor*opacity_factor;"
    "        opacity_factor =  1.0;"
    "      }"
    "      colorindex = clamp((tempval-temperature_min)/(temperature_max-temperature_min),0.0,1.0);"
    "      color_val = texture1D(smokecolormap,colorindex).rgb;"
    "      if(colorindex>0.5){"
    "        soot_val *= fire_opacity_factor;"
    "        in_fire=1;"
    "      };"
    "    }"
    "    else{"
    "      color_val = vec3(0.0,0.0,0.0);"
    "    }"
#ifndef pp_GPUDEPTH
    //  block_val  0.5  block_val2
    //  0.0        x     dstep
    //  x = dstep*(.5-block_val)/(block_val2-block_val)
    "    if(block_val2<0.5){"
    "      in_block=1;"
    "      dstep *= (0.5-block_val)/(block_val2-block_val);"
    "    }"
#endif
    "    taui = exp(-mass_extinct*soot_val*dstep);"
    "    alphai = 1.0 - taui;"
    "    taun *= taui;"
    "    alphan = 1.0-taun;"
    "    color_total += alphai*taun*color_val*opacity_factor;"
    "    cos_angle=0.0;"
    "    if(use_light==1){"
    "      fourpi=16.0*atan(1.0);"
    "      uvec=eyepos-fragpos;"
    "      if(light_type==0){"
    "        vvec=light_position-fragpos;"
    "      }"
    "      else{"
    "        vvec=light_position;"
    "      }"
    "      if(scatter_type!=0){"
    "        cos_angle=dot(uvec,vvec)/(length(uvec)*length(vvec));"
    "        cos_angle=clamp(cos_angle,-1.0,1.0);"
    "      }"
    "      if(scatter_type==0){"
    "        scatter_fraction=1.0/fourpi;"
    "      }"
    "      else if(scatter_type==1){"
    "        scatter_fraction=(1.0-scatter_param*scatter_param)/(pow(1.0+scatter_param*scatter_param-2.0*scatter_param*cos_angle,1.5)*fourpi);"
    "      }"
    "      else{"
    "        scatter_fraction=(1.0-scatter_param*scatter_param)/(pow(1.0+scatter_param*cos_angle,2.0)*fourpi);"
    "      }"
    "      light_fraction = texture3D(light_texture,position);"
    "      light_factor = alphai*light_intensity*light_fraction*scatter_fraction;"
    "      color_total += alphai*taun*light_factor*light_color/255.0;"
    "    }"
    "    if(vol_adaptive==1&&factor>factor0){"
    "      if(abs(tempval-last_tempval)>1.0&&dfactor>0.1/float(n_iter)){"
    "        dfactor/= 2.0;"
    "        dstep /= 2.0;"
    "      }"
    "      if(abs(tempval-last_tempval)<0.5&&dfactor+0.01<dfactor0){"
    "        dfactor *= 2.0;"
    "        dstep *= 2.0;"
    "      }"
    "    }"
    "    factor+=dfactor;"
#ifndef pp_GPUDEPTH
    "    if(block_val2<0.5)break;"
#endif
    "    if(in_block==1){"
    "      break;"
    "    }"
    "  }"
    //"  maxcolor=max(max(color_total.r,color_total.g),color_total.b);"
    //"  color_total/=maxcolor;"
    //"  alphan*=maxcolor;"
    //"  alphan=clamp(alphan,0.0,1.0);"
    "  if(volbw==1){"
    "    gray=color2bw(color_total);"
    "    color_total=vec3(gray,gray,gray);"
    "  }"
    "  if(alphan>0.0){"
    "    gl_FragColor = vec4(color_total/alphan,alphan);"
    "  }"
    "  else{"
    "    gl_FragColor = vec4(0.0,0.0,0.0,0.0);"
    "  }"
    "}" // end of main
  };

  const GLchar *VertexShaderSource[]={
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position = ftransform();"
    "}"
  };

  vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShaderARB(vert_shader);

  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShaderARB(frag_shader);

#ifdef _DEBUG
  PrintInfoLog(vert_shader);
  PrintInfoLog(frag_shader);
#endif

  p_volsmoke = glCreateProgramObjectARB();
  glAttachObjectARB(p_volsmoke,vert_shader);
  glAttachObjectARB(p_volsmoke,frag_shader);

  glLinkProgram(p_volsmoke);
  glGetObjectParameterivARB(p_volsmoke,GL_OBJECT_LINK_STATUS_ARB,&error_code);
#ifdef _DEBUG
  PRINTF("  Volume Smoke shader completion code:");
  switch(error_code){
  case GL_INVALID_VALUE:
    PRINTF(" INVALID VALUE\n");
    break;
  case GL_INVALID_OPERATION:
    PRINTF(" INVALID OPERATION\n");
    break;
  case GL_INVALID_ENUM:
    PRINTF(" INVALID ENUM\n");
    break;
  case LINK_BAD:
    PRINTF(" Link failed\n");
    break;
  case LINK_GOOD:
    PRINTF(" Link succeeded\n");
    break;
  default:
    PRINTF(" unknown error\n");
    break;
  }
  PrintInfoLog(p_volsmoke);
#endif
  GPUvol_inside = glGetUniformLocation(p_volsmoke,"inside");
  GPUvol_eyepos = glGetUniformLocation(p_volsmoke,"eyepos");
#ifdef pp_GPUDEPTH
  GPUvol_depthtexture = glGetUniformLocation(p_volsmoke,"depthtexture");
  GPUvol_screensize = glGetUniformLocation(p_volsmoke,"screensize");
  GPUvol_nearfar = glGetUniformLocation(p_volsmoke,"nearfar");
#endif
  GPUvol_voltemp_offset = glGetUniformLocation(p_volsmoke, "voltemp_offset");
  GPUvol_voltemp_factor = glGetUniformLocation(p_volsmoke, "voltemp_factor");
  GPUvol_block_volsmoke = glGetUniformLocation(p_volsmoke,"block_volsmoke");
  GPUvol_dcell = glGetUniformLocation(p_volsmoke,"dcell");
  GPUvol_dcell3 = glGetUniformLocation(p_volsmoke,"dcell3");
  GPUvol_xyzmaxdiff = glGetUniformLocation(p_volsmoke,"xyzmaxdiff");
  GPUvol_gpu_vol_factor = glGetUniformLocation(p_volsmoke,"gpu_vol_factor");
  GPUvol_fire_opacity_factor = glGetUniformLocation(p_volsmoke,"fire_opacity_factor");
  GPUvol_vol_adaptive = glGetUniformLocation(p_volsmoke, "vol_adaptive");
  GPUvol_mass_extinct = glGetUniformLocation(p_volsmoke,"mass_extinct");
  GPUvol_volbw = glGetUniformLocation(p_volsmoke,"volbw");
  GPUvol_temperature_min = glGetUniformLocation(p_volsmoke,"temperature_min");
  GPUvol_temperature_cutoff = glGetUniformLocation(p_volsmoke,"temperature_cutoff");
  GPUvol_temperature_max = glGetUniformLocation(p_volsmoke,"temperature_max");
  GPUvol_boxmin = glGetUniformLocation(p_volsmoke,"boxmin");
  GPUvol_boxmax = glGetUniformLocation(p_volsmoke,"boxmax");
  GPUvol_soot_density = glGetUniformLocation(p_volsmoke,"soot_density_texture");
  GPUvol_blockage = glGetUniformLocation(p_volsmoke,"blockage_texture");
  GPUvol_fire = glGetUniformLocation(p_volsmoke,"fire_texture");

  GPUvol_use_light = glGetUniformLocation(p_volsmoke, "use_light");
  GPUvol_light_color = glGetUniformLocation(p_volsmoke, "light_color");
  GPUvol_light_intensity = glGetUniformLocation(p_volsmoke, "light_intensity");
  GPUvol_scatter_param = glGetUniformLocation(p_volsmoke, "scatter_param");
  GPUvol_light = glGetUniformLocation(p_volsmoke,"light_texture");
  GPUvol_scatter_param = glGetUniformLocation(p_volsmoke, "scatter_param");
  GPUvol_light_position = glGetUniformLocation(p_volsmoke, "light_position");
  GPUvol_light_type = glGetUniformLocation(p_volsmoke, "light_type");
  GPUvol_scatter_type_glui = glGetUniformLocation(p_volsmoke, "scatter_type");

  GPUvol_havefire = glGetUniformLocation(p_volsmoke,"havefire");
  GPUvol_smokecolormap = glGetUniformLocation(p_volsmoke,"smokecolormap");
  GPUvol_drawsides = glGetUniformLocation(p_volsmoke,"drawsides");

  if(error_code!=1)return error_code;
  return error_code;
}

/* ------------------ SetSmokeShaders ------------------------ */

int SetSmokeShaders(){
  GLhandleARB vert_shader, frag_shader;
  GLint error_code;

  const GLchar *FragmentShaderSource[]={
    "varying vec4 newcolor;"
    "void main(){"
    "  gl_FragColor = newcolor;"
    "}"
  };

  const GLchar *VertexShaderSource[]={
    "uniform sampler1D smokecolormap;"
    "uniform float hrrpuv_max_smv, hrrpuv_cutoff;"
    "uniform float aspectratio, smoke3d_rthick, fire_alpha;"
    "uniform int have_smoke, adjustalphaflag;"

    "attribute float hrr, smoke_alpha;"

    "varying vec4 newcolor;"
    "void main(){"
    "  float alpha,r;"
    "  float term1, term2, term3, term4;"
    "  vec4 hrrcolor,smokecolor;"
    "  float colorindex;"
    "  float hrrlocal;"

//    f(alpha) = 1 - (1-alpha)^r = f(0) + f'(0)alpha + f''(0)alpha^2/2 + f'''(0)alpha^3/6 + ...
//    f(0)    = 0
//    f'(0)   = r
//    f''(0)  = -r(r-1)
//    f'''(0) = r(r-1)(r-2)
//    f''''(0)=-r(r-1)(r-2)(r-3)
    "  alpha=0.0;"
    "  if(have_smoke==1){"
    "    alpha=smoke_alpha/255.0;"
    "    if(adjustalphaflag==1||adjustalphaflag==3){"
    "      r=aspectratio;"
    "      term1 = alpha*r;"
    "      term2 = -term1*alpha*(r-1.0)/2.0;"
    "      term3 = -term2*alpha*(r-2.0)/3.0;"
    "      term4 = -term3*alpha*(r-3.0)/4.0;"
    "      alpha = term1+term2+term3+term4;"
    "    }"
    "    alpha /= smoke3d_rthick;"
    "  }"
    "  hrrlocal=(hrr/254.0)*hrrpuv_max_smv;"
    "  if(hrrlocal>hrrpuv_cutoff){"
    "    colorindex=0.51+((hrrlocal-hrrpuv_cutoff)/(hrrpuv_max_smv-hrrpuv_cutoff))/2.0;"
    "    colorindex=clamp(colorindex,0.5,1.0);"
    "    alpha=fire_alpha/255.0;"
    "    hrrcolor = texture1D(smokecolormap,colorindex);"
    "    newcolor=vec4(vec3(hrrcolor),alpha);"
    "  }"
    "  else{"
    "    colorindex=0.5*hrrlocal/hrrpuv_cutoff;"
    "    colorindex=clamp(colorindex,0.0,0.49);"
    "    smokecolor = texture1D(smokecolormap,colorindex);"
    "    newcolor=vec4(vec3(smokecolor),alpha);"
    "  }"
    "  gl_Position = ftransform();"
    "}"
};

  vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShaderARB(vert_shader);

  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShaderARB(frag_shader);

#ifdef _DEBUG
  PrintInfoLog(vert_shader);
  PrintInfoLog(frag_shader);
#endif

  p_smoke = glCreateProgramObjectARB();
  glAttachObjectARB(p_smoke,vert_shader);
  glAttachObjectARB(p_smoke,frag_shader);

  glLinkProgram(p_smoke);
  glGetObjectParameterivARB(p_smoke,GL_OBJECT_LINK_STATUS_ARB,&error_code);
#ifdef _DEBUG
  PRINTF("  Smoke shader completion code:");
  switch(error_code){
  case GL_INVALID_VALUE:
    PRINTF(" INVALID VALUE\n");
    break;
  case GL_INVALID_OPERATION:
    PRINTF(" INVALID OPERATION\n");
    break;
  case GL_INVALID_ENUM:
    PRINTF(" INVALID ENUM\n");
    break;
  case LINK_BAD:
    PRINTF(" Link failed\n");
    break;
  case LINK_GOOD:
    PRINTF(" Link succeeded\n");
    break;
  default:
    PRINTF(" unknown error\n");
    break;
  }
  PrintInfoLog(p_smoke);
#endif
  if(error_code!=1)return error_code;
  GPU_hrrpuv_max_smv = glGetUniformLocation(p_smoke,"hrrpuv_max_smv");
  GPU_hrrpuv_cutoff =  glGetUniformLocation(p_smoke,"hrrpuv_cutoff");
  GPU_fire_alpha =     glGetUniformLocation(p_smoke,"fire_alpha");
  GPU_smokecolormap =  glGetUniformLocation(p_smoke,"smokecolormap");
  GPU_smoke3d_rthick = glGetUniformLocation(p_smoke,"smoke3d_rthick");
  GPU_have_smoke =     glGetUniformLocation(p_smoke,"have_smoke");
  GPU_aspectratio =    glGetUniformLocation(p_smoke,"aspectratio");
  GPU_adjustalphaflag =glGetUniformLocation(p_smoke,"adjustalphaflag");

  GPU_hrr =            glGetAttribLocation(p_smoke,"hrr");
  GPU_smokealpha =     glGetAttribLocation(p_smoke,"smoke_alpha");
  return error_code;

}


/* ------------------ Load3DSliceShaders ------------------------ */

void Load3DSliceShaders(void){
  glUseProgramObjectARB(p_3dslice);
}

/* ------------------ LoadZoneSmokeShaders ------------------------ */

void LoadZoneSmokeShaders(void){
  glUseProgramObjectARB(p_zonesmoke);
}

/* ------------------ LoadSmokeShaders ------------------------ */

void LoadSmokeShaders(void){
  glUseProgramObjectARB(p_smoke);
}

/* ------------------ LoadVolsmokeShaders ------------------------ */

void LoadVolsmokeShaders(void){
  glUseProgramObjectARB(p_volsmoke);
}

/* ------------------ UnloadShaders ------------------------ */

void UnLoadShaders(void){
  glUseProgramObjectARB(0);
}

/* ------------------ InitShaders ------------------------ */

int InitShaders(void){
  GLenum err;

  gpuactive=0;
  usegpu=0;
  if(opengl_version<200){
    PRINTF("   Smokeview is running on a system using OpenGL %s\n",opengl_version_label);
    PRINTF("   OpenGL 2.0 or later is required to use the GPU.\n");
    PRINTF("   GPU smoke shader not supported.\n");
    return 1;
  }

  if(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader){
    if(SetSmokeShaders()==1){
#ifdef _DEBUG
      PRINTF("   GPU smoke shader successfully compiled, linked and loaded.\n");
#endif
      gpuactive=1;
      err=0;
    }
    else{
      PRINTF("   *** GPU smoke shader failed to load.\n");
      usegpu=0;
      err=1;
    }
    if(SetVolSmokeShaders()==1){
#ifdef _DEBUG
      PRINTF("   GPU smoke Volume shader successfully compiled, linked and loaded.\n");
#endif
      gpuactive=1;
      err=0;
    }
    else{
      PRINTF("   *** GPU smoke volume shader failed to load.\n");
      usegpu=0;
      err=1;
    }
    if(Set3DSliceShaders()==1){
#ifdef _DEBUG
      PRINTF("   GPU 3d slice shader successfully compiled, linked and loaded.\n");
#endif
      gpuactive=1;
      err=0;
    }
    else{
      PRINTF("   *** GPU 3d slice shader failed to load.\n");
      usegpu=0;
      err=1;
    }
    if(err==0){
      if(SetZoneSmokeShaders()==1){
#ifdef _DEBUG
        PRINTF("   GPU zone smoke shader successfully compiled, linked and loaded.\n");
#endif
        gpuactive=1;
        err=0;
      }
      else{
        PRINTF("   *** GPU zone smoke shader failed to load.\n");
        usegpu=0;
        err=1;
      }
    }
  }
  else{
    PRINTF("   *** GPU smoke shader not supported.\n");
    usegpu=0;
    err=1;
  }
  return err;
}

#ifdef pp_GPUDEPTH
/* ------------------ CreateDepthTexture ------------------------ */

void CreateDepthTexture( void ){
  if( depthtexture_id!=0 ){
    glDeleteTextures( 1, &depthtexture_id );
    depthtexture_id = 0;
  }

  glActiveTexture(GL_TEXTURE4);
  glGenTextures(1, &depthtexture_id);
  glBindTexture(GL_TEXTURE_2D, depthtexture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glActiveTexture(GL_TEXTURE0);

}

/* ------------------ GetDepthTexture ------------------------ */

void GetDepthTexture( void ){
  if( depthtexture_id==0 ) CreateDepthTexture();
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, depthtexture_id);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, 0, 0, screenWidth, screenHeight);
  glActiveTexture(GL_TEXTURE0);
}
#endif

#endif
