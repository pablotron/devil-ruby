#include <ruby.h>
#include <IL/il.h>
#include <IL/ilu.h>

#define DEVIL_VERSION "0.1.0"
#define UNUSED(a) ((void) (a))
#define DEF_CONST(a,b,c,d)          \
  do {                              \
    VALUE v = INT2FIX(d);           \
    rb_define_const(a, c, v);       \
    rb_define_const(a, b "_" c, v); \
  } while (0)

#define DEF_METH(a,b,c,d,e)              \
  do {                                   \
    rb_define_singleton_method(a,b,d,e); \
    rb_define_singleton_method(a,c,d,e); \
  } while (0)

/* utility functions */
static char *get_ext(char *str);

static VALUE mDevil,
             mIl,
             mIlu,
             load_procs,
             save_procs;

/*
 * Set the active image.
 *
 * Aliases:
 *   DevIL::IL::active_image
 *   DevIL::IL::ActiveImage
 *
 * Examples:
 *   DevIL::IL::active_image im_id
 *   DevIL::IL::ActiveImage im_id
 * 
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00008.htm
 * 
 */
static VALUE il_active_im(VALUE self, VALUE num) {
  return ilActiveImage(NUM2INT(num)) ? Qtrue : Qfalse;
}

/*
 * Set the active layer.
 *
 * Aliases:
 *   DevIL::IL::active_layer
 *   DevIL::IL::ActiveLayer
 *
 * Examples:
 *   DevIL::IL::active_layer layer_id
 *   DevIL::IL::ActiveLayer layer_id
 *
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00190.htm
 * 
 */
static VALUE il_active_layer(VALUE self, VALUE num) {
  return ilActiveLayerr(NUM2INT(num)) ? Qtrue : Qfalse;
}

/*
 * Set the active mipmap.
 *
 * Aliases:
 *   DevIL::IL::active_mipmap
 *   DevIL::IL::ActiveMipmap
 *
 * Examples:
 *   DevIL::IL::active_mipmap mm_id
 *   DevIL::IL::ActiveMipmap mm_id
 * 
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00009.htm
 * 
 */
static VALUE il_active_mipmap(VALUE self, VALUE num) {
  return ilActiveMipmap(NUM2INT(num)) ? Qtrue : Qfalse;
}

/*
 * Apply pal to current image.
 *
 * Aliases:
 *   DevIL::IM::apply_pal
 *   DevIL::IM::ApplyPal
 *
 * Examples:
 *   DevIL::IL::apply_path path/to/pal
 *   DevIL::IL::ApplyPath path/to/pal
 *
 */
static VALUE il_apply_pal(VALUE self, VALUE path) {
  return ilApplyPal(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

/*
 * Apply color profile to current image.
 *
 * Aliases:
 *   DevIL::IL::apply_profile
 *   DevIL::IL::ApplyProfile
 *
 * Examples:
 *   DevIL::IL::apply_profile in_p, out_p
 *   DevIL::IL::ApplyProfile in_p, out_p
 *   
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00208.htm
 *
 */
static VALUE il_apply_profile(VALUE self, VALUE in, VALUE out) {
  return ilApplyProfile(RSTRING(in)->ptr, RSTRING(out)->ptr) ? Qtrue : Qfalse;
}

/*
 * Select the image to work on.
 *
 * Aliases:
 *   DevIL::IL::bind_image
 *   DevIL::IL::BindImage
 *
 * Examples:
 *   DevIL::IL::bind_image 4
 *   DevIL::IL::BindImage 4
 *
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00013.htm
 *
 */
static VALUE il_bind_im(VALUE self, VALUE num) {
  ilBindImage(NUM2INT(num));
  return Qnil;
}

/*
 * Blit (copy) from one image to another.
 *
 * Aliases:
 *   DevIL::IL::blit
 *   DevIL::IL::Blit
 *
 * Example:
 *   DevIL::IL::blit src_im, dest_x, dest_y,
 *                   src_x, src_y, src_z,
 *                   width, height, depth
 *
 */
static VALUE il_blit(VALUE self, VALUE source, VALUE dest_x, VALUE dest_y, VALUE dest_z, VALUE src_x, VALUE src_y, VALUE src_z, VALUE width, VALUE height, VALUE depth) {
  return ilBlit(NUM2INT(source), NUM2INT(dest_x), NUM2INT(dest_y), NUM2INT(dest_z), NUM2INT(src_x), NUM2INT(src_y), NUM2INT(src_z), NUM2INT(width), NUM2INT(height), NUM2INT(depth)) ? Qtrue : Qfalse;
}

/*
 * Set the current clearing color
 *
 * Aliases:
 *   DevIL::IL::clear_color
 *   DevIL::IL::ClearColor
 *   DevIL::IL::clear_colour
 *   DevIL::IL::ClearColour
 *
 * Example:
 *   DevIL::IL::clear_color 128, 128, 128, 128
 *
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00124.htm
 *
 */
static VALUE il_clear_color(VALUE self, VALUE red, VALUE blue, VALUE green, VALUE alpha) {
  ilClearColor(NUM2DBL(red), NUM2DBL(blue), NUM2DBL(green), NUM2DBL(alpha));
  return Qnil;
}

/* 
 * Clear the current image.
 * 
 * Aliases:
 *   DevIL::IL::clear_image
 *   DevIL::IL::ClearImage
 *
 * Example:
 *   DevIL::IL::clear_image
 * 
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00014.htm
 *
 */
static VALUE il_clear_im(VALUE self) {
  return ilClearImage() ? Qtrue : Qfalse;
}

/* 
 * Clone (copy) the current image.
 * 
 * Aliases:
 *   DevIL::IL::clone_image
 *   DevIL::IL::CloneImage
 *
 * Example:
 *   new_im = DevIL::IL::clone_image
 * 
 * See Also:
 *   http://openil.sourceforge.net/docs/il/f00192.htm
 *
 */
static VALUE il_clone_cur_im(VALUE self) {
  return INT2FIX(ilCloneCurImage());
}

static VALUE il_compress_func(VALUE self, VALUE num) {
  return ilCompressFunc(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_convert_im(VALUE self, VALUE dest_fmt, VALUE dest_type) {
  return ilConvertImage(NUM2INT(dest_fmt), NUM2INT(dest_type)) ? Qtrue : Qfalse;
}

static VALUE il_convert_pal(VALUE self, VALUE dest_fmt) {
  return ilConvertPal(NUM2INT(dest_fmt)) ? Qtrue : Qfalse;
}

static VALUE il_copy_im(VALUE self, VALUE src) {
  return ilCopyImage(NUM2INT(src)) ? Qtrue : Qfalse;
}

static VALUE il_copy_pixels(VALUE self, VALUE xo, VALUE yo, VALUE zo, VALUE w, VALUE h, VALUE d, VALUE fmt, VALUE type, VALUE data) {
  return INT2FIX(ilCopyPixels(NUM2INT(xo), NUM2INT(yo), NUM2INT(zo), NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(fmt), NUM2INT(type), RSTRING(data)->ptr));
}

static VALUE il_create_sub_im(VALUE self, VALUE type, VALUE num) {
  return INT2FIX(ilCreateSubImage(NUM2INT(type), NUM2INT(num)));
}

static VALUE il_default_im(VALUE self) {
  return ilDefaultImage() ? Qtrue : Qfalse;
}

static VALUE il_delete_ims(int argc, VALUE *argv, VALUE self) {
  ILuint *ims;
  int i;

  if (!argc)
    return;
  if ((ims = malloc(sizeof(ILuint) * argc)) == NULL)
    return;

  for (i = 0; i < argc; i++)
    ims[i] = NUM2INT(argv[i]);

  ilDeleteImages(argc, ims);
  free(ims);
}

static VALUE il_disable(VALUE self, VALUE num) {
  return ilDisable(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_enable(VALUE self, VALUE num) {
  return ilEnable(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_format_func(VALUE self, VALUE num) {
  return ilFormatFunc(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_gen_ims(int argc, VALUE *argv, VALUE self) {
  ILuint *ims;
  int i;

  if (!argc)
    return;
  if ((ims = malloc(sizeof(ILuint) * argc)) == NULL)
    return;

  for (i = 0; i < argc; i++)
    ims[i] = NUM2INT(argv[i]);

  ilGenImages(argc, ims);
  free(ims);
}

static VALUE il_get_alpha(VALUE self, VALUE type) {
  int t = NUM2INT(type); 
  VALUE ret = Qnil;
  ILubyte *a_ret;

  a_ret = ilGetAlpha(t);
  switch (t) {
    case IL_BYTE:
    case IL_UNSIGNED_BYTE:
    case IL_SHORT:
    case IL_UNSIGNED_SHORT:
    case IL_INT:
    case IL_UNSIGNED_INT:
      /* is this correct? */
      ret = NUM2INT(*a_ret);
      break;
    case IL_FLOAT:
    case IL_DOUBLE:
      ret = rb_float_new(*a_ret);
      break;
    default:
      rb_raise(rb_eException, "Invalid type: %d.", t);
  }

  return ret;
}

static VALUE il_get_bool(VALUE self, VALUE num) {
  return ilGetBoolean(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_get_data(VALUE self) {
  /* TODO: finish this function */
  return Qnil;
}

static VALUE il_get_dxtc_data(VALUE self, VALUE buf, VALUE fmt) {
  return NUM2INT(ilGetDXTCData(RSTRING(buf)->ptr, RSTRING(buf)->len, NUM2INT(fmt)));
}

static VALUE il_get_err(VALUE self) {
  return NUM2INT(ilGetError());
}

static VALUE il_get_int(VALUE self, VALUE num) {
  return NUM2INT(ilGetInteger(INT2FIX(num)));
}

static VALUE il_get_lump_pos(VALUE self) {
  return NUM2INT(ilGetLumpPos());
}

static VALUE il_get_palette(VALUE self) {
  /* TODO: finish this function */
  return Qnil;
}

static VALUE il_get_string(VALUE self, VALUE num) {
  return rb_str_new2(ilGetString(NUM2INT(num)));
}

static VALUE il_hint(VALUE self, VALUE target, VALUE mode) {
  ilHint(NUM2INT(target), NUM2INT(mode));
  return Qnil;
}

static VALUE il_is_disabled(VALUE self, VALUE mode) {
  return ilIsDisabled(NUM2INT(mode)) ? Qtrue : Qfalse;
}

static VALUE il_is_enabled(VALUE self, VALUE mode) {
  return ilIsEnabled(NUM2INT(mode)) ? Qtrue : Qfalse;
}

static VALUE il_is_im(VALUE self, VALUE im) {
  return ilIsImage(NUM2INT(im)) ? Qtrue : Qfalse;
}

static VALUE il_is_valid(VALUE self, VALUE type, VALUE path) {
  return ilIsValid(NUM2INT(type), RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_is_valid_f(VALUE self, VALUE type, VALUE file) {
  /* TODO: finish this method */
  return Qnil;
}

static VALUE il_is_valid_l(VALUE self, VALUE type, VALUE buf) {
  /* TODO: finish this method */
  return ilIsValidL(NUM2INT(type), RSTRING(buf)->ptr, RSTRING(buf)->len) ? Qtrue : Qfalse;
}

static VALUE il_key_color(VALUE self, VALUE red, VALUE green, VALUE blue, VALUE alpha) {
  ilKeyColor(NUM2DBL(red), NUM2DBL(green), NUM2DBL(blue), NUM2DBL(alpha));
  return Qnil;
}

static VALUE il_load(VALUE self, VALUE type, VALUE path) {
  return ilLoad(NUM2INT(type), RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_load_f(VALUE self, VALUE type, VALUE path) {
  /* TODO: finish this method */
  return Qnil;
}

static VALUE il_load_l(VALUE self, VALUE type, VALUE buf) {
  return ilLoadL(NUM2INT(type), RSTRING(buf)->ptr, RSTRING(buf)->len) ? Qtrue : Qfalse;
}

static VALUE il_load_im(VALUE self, VALUE path) {
  return ilLoadImage(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_load_pal(VALUE self, VALUE path) {
  return ilLoadPal(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_origin_func(VALUE self, VALUE mode) {
  return ilOriginFunc(NUM2INT(mode)) ? Qtrue : Qfalse;
}

static VALUE il_overlay_im(VALUE self, VALUE src, VALUE x, VALUE y, VALUE z) {
  return ilOverlayImage(NUM2INT(src), NUM2INT(x), NUM2INT(y), NUM2INT(z)) ? Qtrue : Qfalse;
}

static VALUE il_pop_attrib(VALUE self) {
  ilPopAttrib();
  return Qnil;
}

static VALUE il_push_attrib(VALUE self, VALUE bits) {
  ilPushAttrib(NUM2INT(bits));
  return Qnil;
}

static VALUE il_register_format(VALUE self, VALUE fmt) {
  ilRegisterFormat(NUM2INT(fmt));
  return Qnil;
}

static ILenum load_cb(const ILstring *str) {
  char *ext;
  VALUE proc;
  static ID id = 0;
  
  if (!id)
    id = rb_intern("Proc::call");

  if ((ext = get_ext((char*) str)) != NULL) {
    if ((proc = rb_hash_get(load_procs, rb_str_new2(ext))) != Qnil) {
      VALUE ary;
      ary = rb_ary_new();
      rb_ary_push(ary, rb_str_new2((char*) str));
      return NUM2INT(rb_func_call(proc, id, 1, ary));
    }
  }

  return 0;
}

static VALUE il_register_load(VALUE self, VALUE ext, VALUE proc) {
  rb_hash_set(load_procs, ext, proc);
  return ilRegisterLoad((ILstring) RSTRING(ext)->ptr, (IL_LOADPROC) load_cb) ? Qtrue : Qfalse;
}

static VALUE il_register_mipnum(VALUE self, VALUE num) {
  return ilRegisterMipnum(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_register_num_ims(VALUE self, VALUE num) {
  return ilRegisterNumImages(NUM2INT(num)) ? Qtrue : Qfalse;
}

static VALUE il_register_origin(VALUE self, VALUE num) {
  ilRegisterOrigin(NUM2INT(num));
  return Qnil;
}

static VALUE il_register_pal(VALUE self, VALUE buf, VALUE type) {
  ilRegisterPal(RSTRING(buf)->ptr, RSTRING(buf)->len, NUM2INT(type));
  return Qnil;
}

static ILenum save_cb(const ILstring *str) {
  char *ext;
  VALUE proc;
  static ID id = 0;
  
  if (!id)
    id = rb_intern("Proc::call");

  if ((ext = get_ext((char*) str)) != NULL) {
    if ((proc = rb_hash_get(save_procs, rb_str_new2(ext))) != Qnil) {
      VALUE ary;
      ary = rb_ary_new();
      rb_ary_push(ary, rb_str_new2((char*) str));
      return NUM2INT(rb_func_call(proc, id, 1, ary));
    }
  }

  return 0;
}

static VALUE il_register_save(VALUE self, VALUE ext, VALUE proc) {
  rb_hash_set(save_procs, ext, proc);
  return ilRegisterSave((ILstring) RSTRING(ext)->ptr, (IL_SAVEPROC) save_cb) ? Qtrue : Qfalse;
}

static VALUE il_register_type(VALUE self, VALUE num) {
  ilRegisterType(NUM2INT(num));
  return Qnil;
}

static VALUE il_remove_load(VALUE self, VALUE ext) {
  /* TODO: rb_hash_set(save_procs, ext, proc); */
  return ilRemoveLoad((ILstring) RSTRING(ext)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_remove_save(VALUE self, VALUE ext) {
  /* TODO: rb_hash_set(save_procs, ext, proc); */
  return ilRemoveSave((ILstring) RSTRING(ext)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_reset_mem(VALUE self) {
  ilResetMemory();
  return Qnil;
}

static VALUE il_reset_read(VALUE self) {
  ilResetRead();
  return Qnil;
}

static VALUE il_reset_write(VALUE self) {
  ilResetWrite();
  return Qnil;
}

static VALUE il_save(VALUE self, VALUE type, VALUE path) {
  return ilSave(NUM2INT(type), RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_save_f(VALUE self, VALUE file) {
  /* TODO: me! */
  return Qnil;
}

static VALUE il_save_im(VALUE self, VALUE path) {
  return ilSaveImage(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_save_l(VALUE self, VALUE type, VALUE buf) {
  return NUM2INT(ilSaveL(NUM2INT(type), RSTRING(buf)->ptr, RSTRING(buf)->len));
}

static VALUE il_save_pal(VALUE self, VALUE path) {
  return ilSavePal(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_set_data(VALUE self, VALUE buf) {
  return ilSetData(RSTRING(buf)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_set_duration(VALUE self, VALUE dur) {
  return ilSetDuration(NUM2INT(dur)) ? Qtrue : Qfalse;
}

static VALUE il_set_int(VALUE self, VALUE mode, VALUE val) {
  ilSetInteger(NUM2INT(mode), NUM2INT(val));
  return Qnil;
}

static VALUE il_set_mem(VALUE self, VALUE m_alloc, VALUE m_free) {
  /* TODO: ilSetMemory(); */
  return Qnil;
}

static VALUE il_set_pixels(VALUE self, VALUE xo, VALUE yo, VALUE zo, VALUE w, VALUE h, VALUE d, VALUE fmt, VALUE type, VALUE data) {
  ilSetPixels(NUM2INT(xo), NUM2INT(yo), NUM2INT(zo), NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(fmt), NUM2INT(type), RSTRING(data)->ptr);
  return Qnil;
}

static VALUE il_set_read(VALUE self, VALUE o, VALUE c, VALUE p, VALUE t, VALUE w) {
  /* TODO: this! */
  return Qnil;
}

static VALUE il_set_string(VALUE self, VALUE mode, VALUE str) {
  ilSetString(NUM2INT(mode), RSTRING(str)->ptr);
  return Qnil;
}

static VALUE il_set_write(VALUE self, VALUE o, VALUE c, VALUE p, VALUE t, VALUE w) {
  /* TODO: this! */
  return Qnil;
}

static VALUE il_shutdown(VALUE self) {
  ilShutdown();
  return Qnil;
}

static VALUE il_tex_im(VALUE self, VALUE w, VALUE h, VALUE d, VALUE bpp, VALUE fmt, VALUE type, VALUE data) {
  return ilTexImage(NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(bpp), NUM2INT(fmt), NUM2INT(type), RSTRING(data)->ptr) ? Qtrue : Qfalse;
}

static VALUE il_type_func(VALUE self, VALUE mode) {
  return lType(NUM2INT(mode)) ? Qtrue : Qfalse;
}

static VALUE il_load_data(VALUE self, VALUE path, VALUE w, VALUE h, VALUE d, VALUE bpp) {
  return ilLoadData(RSTRING(path)->ptr, NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(bpp)) ? Qtrue : Qfalse;
}

static VALUE il_load_data_f(VALUE self, VALUE file, VALUE w, VALUE h, VALUE d, VALUE bpp) {
  /* TODO: return ilLoadDataF(???, NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(bpp)) ? Qtrue : Qfalse; */
return Qnil;
}

static VALUE il_load_data_l(VALUE self, VALUE buf, VALUE w, VALUE h, VALUE d, VALUE bpp) {
  return ilLoadDataL(RSTRING(buf)->ptr, RSTRING(buf)->len, NUM2INT(w), NUM2INT(h), NUM2INT(d), NUM2INT(bpp)) ? Qtrue : Qfalse;
}

static VALUE il_save_data(VALUE self, VALUE path) {
  return ilSaveData(RSTRING(path)->ptr) ? Qtrue : Qfalse;
}

/**********************/
/* define ILU methods */
/**********************/
static VALUE ilu_alienify(VALUE self) {
  return iluAlienify() ? Qtrue : Qfalse;
}

static VALUE ilu_blur_avg(VALUE self, VALUE iter) {
  return iluBlurAvg(NUM2INT(iter)) ? Qtrue : Qfalse;
}

static VALUE ilu_blur_gaussian(VALUE self, VALUE iter) {
  return iluBlurGaussian(NUM2INT(iter)) ? Qtrue : Qfalse;
}

static VALUE ilu_build_mipmaps(VALUE self) {
  return iluBuildMipmaps() ? Qtrue : Qfalse;
}

static VALUE ilu_colors_used(VALUE self) {
  return INT2FIX(iluColorsUsed());
}

static VALUE ilu_compare_im(VALUE self, VALUE comp) {
  return iluCompareImage(NUM2INT(comp)) ? Qtrue : Qfalse;
}

static VALUE ilu_contrast(VALUE self, VALUE contrast) {
  return iluContrast(NUM2DBL(contrast)) ? Qtrue : Qfalse;
}

static VALUE ilu_crop(VALUE self, VALUE xo, VALUE yo, VALUE zo, VALUE w, VALUE h, VALUE d) {
  return iluCrop(NUM2INT(xo), NUM2INT(yo), NUM2INT(zo), NUM2INT(w), NUM2INT(h), NUM2INT(d)) ? Qtrue : Qfalse;
}

static VALUE ilu_delete_im(VALUE self, VALUE id) {
  iluDeleteImage(NUM2INT(id));
  return Qnil;
}

static VALUE ilu_edge_detect_e(VALUE self) {
  return iluEdgeDetectE() ? Qtrue : Qfalse;
}

static VALUE ilu_edge_detect_p(VALUE self) {
  return iluEdgeDetectP() ? Qtrue : Qfalse;
}

static VALUE ilu_edge_detect_s(VALUE self) {
  return iluEdgeDetectS() ? Qtrue : Qfalse;
}

static VALUE ilu_emboss(VALUE self) {
  return iluEmboss() ? Qtrue : Qfalse;
}

static VALUE ilu_enlarge_canvas(VALUE self, VALUE w, VALUE h, VALUE d) {
  return iluEnlargeCanvas(NUM2INT(w), NUM2INT(h), NUM2INT(d)) ? Qtrue : Qfalse;
}

static VALUE ilu_enlarge_im(VALUE self, VALUE x, VALUE y, VALUE z) {
  return iluEnlargeImage(NUM2DBL(x), NUM2DBL(y), NUM2DBL(z)) ? Qtrue : Qfalse;
}

static VALUE ilu_equalize(VALUE self) {
  return iluEqualize() ? Qtrue : Qfalse;
}

static VALUE ilu_error_string(VALUE self, VALUE err) {
  return rb_str_new2(iluErrorString(NUM2INT(err)));
}

static VALUE ilu_flip_im(VALUE self) {
  return iluFlipImage() ? Qtrue : Qfalse;
}

static VALUE ilu_gamma_correct(VALUE self, VALUE gamma) {
  return iluGammaCorrect(NUM2DBL(gamma)) ? Qtrue : Qfalse;
}

static VALUE ilu_gen_im(VALUE self) {
  return INT2FIX(iluGenImage());
}

static VALUE ilu_get_im_info(VALUE self) {
  /* TODO: this func! */
  return Qnil;
}

static VALUE ilu_get_int(VALUE self, VALUE mode) {
  return INT2FIX(iluGetInteger(NUM2INT(mode)));
}

static VALUE ilu_get_string(VALUE self, VALUE num) {
  return rb_str_new2(iluGetString(NUM2INT(num)));
}

static VALUE ilu_im_parameter(VALUE self, VALUE name, VALUE param) {
  iluImageParameter(NUM2INT(name), NUM2INT(param));
  return Qnil;
}

static VALUE ilu_invert_alpha(VALUE self) {
  return iluInvertAlpha() ? Qtrue : Qfalse;
}

static VALUE ilu_load_im(VALUE self, VALUE path) {
  return INT2FIX(iluLoadImage(RSTRING(path)->ptr));
}

static VALUE ilu_mirror(VALUE self) {
  return iluMirror() ? Qtrue : Qfalse;
}

static VALUE ilu_negative(VALUE self) {
  return iluNegative() ? Qtrue : Qfalse;
}

static VALUE ilu_noisify(VALUE self, VALUE tol) {
  return iluNoisify(NUM2DBL(tol)) ? Qtrue : Qfalse;
}

static VALUE ilu_pixelize(VALUE self, VALUE size) {
  return iluPixelize(NUM2INT(size)) ? Qtrue : Qfalse;
}

static VALUE ilu_region_fv(VALUE self, VALUE points) {
  /* TODO: this function! */
  return Qnil;
}

static VALUE ilu_region_iv(VALUE self, VALUE points) {
  /* TODO: this function! */
  return Qnil;
}

static VALUE ilu_replace_color(VALUE self, VALUE r, VALUE g, VALUE b, VALUE tol) {
  return iluReplaceColor(NUM2INT(r), NUM2INT(g), NUM2INT(b), NUM2DBL(tol)) ? Qtrue : Qfalse;
}

static VALUE ilu_rotate(VALUE self, VALUE angle) {
  return iluRotate(NUM2DBL(angle)) ? Qtrue : Qfalse;
}

static VALUE ilu_rotate_3d(VALUE self, VALUE x, VALUE y, VALUE z, VALUE a) {
  return iluRotate3D(NUM2DBL(x), NUM2DBL(y), NUM2DBL(z), NUM2DBL(a)) ? Qtrue : Qfalse;
}

static VALUE ilu_saturate_1f(VALUE self, VALUE sat) {
  return iluSaturate1f(NUM2DBL(sat)) ? Qtrue : Qfalse;
}

static VALUE ilu_saturate_4f(VALUE self, VALUE r, VALUE g, VALUE b, VALUE sat) {
  return iluSaturate4f(NUM2DBL(r), NUM2DBL(g), NUM2DBL(b), NUM2DBL(sat)) ? Qtrue : Qfalse;
}

static VALUE ilu_scale(VALUE self, VALUE w, VALUE h, VALUE d) {
  return iluScale(NUM2INT(w), NUM2INT(h), NUM2INT(d)) ? Qtrue : Qfalse;
}

static VALUE ilu_scale_colors(VALUE self, VALUE r, VALUE g, VALUE b) {
  return iluScale(NUM2INT(r), NUM2INT(g), NUM2INT(b)) ? Qtrue : Qfalse;
}

static VALUE ilu_sharpen(VALUE self, VALUE factor, VALUE iter) {
  return iluSharpen(NUM2DBL(factor), NUM2INT(iter)) ? Qtrue : Qfalse;
}

static VALUE ilu_swap_colors(VALUE self) {
  return iluSwapColors() ? Qtrue : Qfalse;
}

static VALUE ilu_wave(VALUE self, VALUE angle) {
  return iluWave(NUM2DBL(angle)) ? Qtrue : Qfalse;
}




static void define_constants(void) {
  DEF_CONST(mIl, "IL", "COLOUR_INDEX", IL_COLOUR_INDEX);
  DEF_CONST(mIl, "IL", "COLOR_INDEX", IL_COLOR_INDEX);
  DEF_CONST(mIl, "IL", "RGB", IL_RGB);
  DEF_CONST(mIl, "IL", "RGBA", IL_RGBA);
  DEF_CONST(mIl, "IL", "BGR", IL_BGR);
  DEF_CONST(mIl, "IL", "BGRA", IL_BGRA);
  DEF_CONST(mIl, "IL", "LUMINANCE", IL_LUMINANCE);
  DEF_CONST(mIl, "IL", "LUMINANCE_ALPHA", IL_LUMINANCE_ALPHA);

  DEF_CONST(mIl, "IL", "BYTE", IL_BYTE);
  DEF_CONST(mIl, "IL", "UNSIGNED_BYTE", IL_UNSIGNED_BYTE);
  DEF_CONST(mIl, "IL", "SHORT", IL_SHORT);
  DEF_CONST(mIl, "IL", "UNSIGNED_SHORT", IL_UNSIGNED_SHORT);
  DEF_CONST(mIl, "IL", "INT", IL_INT);
  DEF_CONST(mIl, "IL", "UNSIGNED_INT", IL_UNSIGNED_INT);
  DEF_CONST(mIl, "IL", "FLOAT", IL_FLOAT);
  DEF_CONST(mIl, "IL", "DOUBLE", IL_DOUBLE);

  DEF_CONST(mIl, "IL", "VENDOR", IL_VENDOR);
  DEF_CONST(mIl, "IL", "LOAD_EXT", IL_LOAD_EXT);
  DEF_CONST(mIl, "IL", "SAVE_EXT", IL_SAVE_EXT);

  /* IL-specific #define's */
  DEF_CONST(mIl, "IL", "VERSION_1_6_1", IL_VERSION_1_6_1);
  DEF_CONST(mIl, "IL", "VERSION", IL_VERSION);

  /* Attribute Bits */
  DEF_CONST(mIl, "IL", "ORIGIN_BIT", IL_ORIGIN_BIT);
  DEF_CONST(mIl, "IL", "FILE_BIT", IL_FILE_BIT);
  DEF_CONST(mIl, "IL", "PAL_BIT", IL_PAL_BIT);
  DEF_CONST(mIl, "IL", "FORMAT_BIT", IL_FORMAT_BIT);
  DEF_CONST(mIl, "IL", "TYPE_BIT", IL_TYPE_BIT);
  DEF_CONST(mIl, "IL", "COMPRESS_BIT", IL_COMPRESS_BIT);
  DEF_CONST(mIl, "IL", "LOADFAIL_BIT", IL_LOADFAIL_BIT);
  DEF_CONST(mIl, "IL", "FORMAT_SPECIFIC_BIT", IL_FORMAT_SPECIFIC_BIT);
  DEF_CONST(mIl, "IL", "ALL_ATTRIB_BITS", IL_ALL_ATTRIB_BITS);

  /* Palette types */
  DEF_CONST(mIl, "IL", "PAL_NONE", IL_PAL_NONE);
  DEF_CONST(mIl, "IL", "PAL_RGB24", IL_PAL_RGB24);
  DEF_CONST(mIl, "IL", "PAL_RGB32", IL_PAL_RGB32);
  DEF_CONST(mIl, "IL", "PAL_RGBA32", IL_PAL_RGBA32);
  DEF_CONST(mIl, "IL", "PAL_BGR24", IL_PAL_BGR24);
  DEF_CONST(mIl, "IL", "PAL_BGR32", IL_PAL_BGR32);
  DEF_CONST(mIl, "IL", "PAL_BGRA32", IL_PAL_BGRA32);

  /* Image types */
  DEF_CONST(mIl, "IL", "TYPE_UNKNOWN", IL_TYPE_UNKNOWN);
  DEF_CONST(mIl, "IL", "BMP", IL_BMP);
  DEF_CONST(mIl, "IL", "CUT", IL_CUT);
  DEF_CONST(mIl, "IL", "DOOM", IL_DOOM);
  DEF_CONST(mIl, "IL", "DOOM_FLAT", IL_DOOM_FLAT);
  DEF_CONST(mIl, "IL", "ICO", IL_ICO);
  DEF_CONST(mIl, "IL", "JPG", IL_JPG);
  DEF_CONST(mIl, "IL", "JFIF", IL_JFIF);
  DEF_CONST(mIl, "IL", "LBM", IL_LBM);
  DEF_CONST(mIl, "IL", "PCD", IL_PCD);
  DEF_CONST(mIl, "IL", "PCX", IL_PCX);
  DEF_CONST(mIl, "IL", "PIC", IL_PIC);
  DEF_CONST(mIl, "IL", "PNG", IL_PNG);
  DEF_CONST(mIl, "IL", "PNM", IL_PNM);
  DEF_CONST(mIl, "IL", "SGI", IL_SGI);
  DEF_CONST(mIl, "IL", "TGA", IL_TGA);
  DEF_CONST(mIl, "IL", "TIF", IL_TIF);
  DEF_CONST(mIl, "IL", "CHEAD", IL_CHEAD);
  DEF_CONST(mIl, "IL", "RAW", IL_RAW);
  DEF_CONST(mIl, "IL", "MDL", IL_MDL);
  DEF_CONST(mIl, "IL", "WAL", IL_WAL);
  DEF_CONST(mIl, "IL", "LIF", IL_LIF);
  DEF_CONST(mIl, "IL", "MNG", IL_MNG);
  DEF_CONST(mIl, "IL", "JNG", IL_JNG);
  DEF_CONST(mIl, "IL", "GIF", IL_GIF);
  DEF_CONST(mIl, "IL", "DDS", IL_DDS);
  DEF_CONST(mIl, "IL", "DCX", IL_DCX);
  DEF_CONST(mIl, "IL", "PSD", IL_PSD);
  DEF_CONST(mIl, "IL", "EXIF", IL_EXIF);
  DEF_CONST(mIl, "IL", "PSP", IL_PSP);
  DEF_CONST(mIl, "IL", "PIX", IL_PIX);
  DEF_CONST(mIl, "IL", "PXR", IL_PXR);
  DEF_CONST(mIl, "IL", "XPM", IL_XPM);

  DEF_CONST(mIl, "IL", "JASC_PAL", IL_JASC_PAL);

  /* Error Types */
  DEF_CONST(mIl, "IL", "NO_ERROR", IL_NO_ERROR);
  DEF_CONST(mIl, "IL", "INVALID_ENUM", IL_INVALID_ENUM);
  DEF_CONST(mIl, "IL", "OUT_OF_MEMORY", IL_OUT_OF_MEMORY);
  DEF_CONST(mIl, "IL", "FORMAT_NOT_SUPPORTED", IL_FORMAT_NOT_SUPPORTED);
  DEF_CONST(mIl, "IL", "INTERNAL_ERROR", IL_INTERNAL_ERROR);
  DEF_CONST(mIl, "IL", "INVALID_VALUE", IL_INVALID_VALUE);
  DEF_CONST(mIl, "IL", "ILLEGAL_OPERATION", IL_ILLEGAL_OPERATION);
  DEF_CONST(mIl, "IL", "ILLEGAL_FILE_VALUE", IL_ILLEGAL_FILE_VALUE);
  DEF_CONST(mIl, "IL", "INVALID_FILE_HEADER", IL_INVALID_FILE_HEADER);
  DEF_CONST(mIl, "IL", "INVALID_PARAM", IL_INVALID_PARAM);
  DEF_CONST(mIl, "IL", "COULD_NOT_OPEN_FILE", IL_COULD_NOT_OPEN_FILE);
  DEF_CONST(mIl, "IL", "INVALID_EXTENSION", IL_INVALID_EXTENSION);
  DEF_CONST(mIl, "IL", "FILE_ALREADY_EXISTS", IL_FILE_ALREADY_EXISTS);
  DEF_CONST(mIl, "IL", "OUT_FORMAT_SAME", IL_OUT_FORMAT_SAME);
  DEF_CONST(mIl, "IL", "STACK_OVERFLOW", IL_STACK_OVERFLOW);
  DEF_CONST(mIl, "IL", "STACK_UNDERFLOW", IL_STACK_UNDERFLOW);
  DEF_CONST(mIl, "IL", "INVALID_CONVERSION", IL_INVALID_CONVERSION);
  DEF_CONST(mIl, "IL", "BAD_DIMENSIONS", IL_BAD_DIMENSIONS);
  DEF_CONST(mIl, "IL", "FILE_READ_ERROR", IL_FILE_READ_ERROR);
  DEF_CONST(mIl, "IL", "FILE_WRITE_ERROR", IL_FILE_WRITE_ERROR);

  DEF_CONST(mIl, "IL", "LIB_GIF_ERROR", IL_LIB_GIF_ERROR);
  DEF_CONST(mIl, "IL", "LIB_JPEG_ERROR", IL_LIB_JPEG_ERROR);
  DEF_CONST(mIl, "IL", "LIB_PNG_ERROR", IL_LIB_PNG_ERROR);
  DEF_CONST(mIl, "IL", "LIB_TIFF_ERROR", IL_LIB_TIFF_ERROR);
  DEF_CONST(mIl, "IL", "LIB_MNG_ERROR", IL_LIB_MNG_ERROR);
  DEF_CONST(mIl, "IL", "UNKNOWN_ERROR", IL_UNKNOWN_ERROR);

  /* Origin Definitions */
  DEF_CONST(mIl, "IL", "ORIGIN_SET", IL_ORIGIN_SET);
  DEF_CONST(mIl, "IL", "ORIGIN_LOWER_LEFT", IL_ORIGIN_LOWER_LEFT);
  DEF_CONST(mIl, "IL", "ORIGIN_UPPER_LEFT", IL_ORIGIN_UPPER_LEFT);
  DEF_CONST(mIl, "IL", "ORIGIN_MODE", IL_ORIGIN_MODE);

  /* Format and Type Mode Definitions */
  DEF_CONST(mIl, "IL", "FORMAT_SET", IL_FORMAT_SET);
  DEF_CONST(mIl, "IL", "FORMAT_MODE", IL_FORMAT_MODE);
  DEF_CONST(mIl, "IL", "TYPE_SET", IL_TYPE_SET);
  DEF_CONST(mIl, "IL", "TYPE_MODE", IL_TYPE_MODE);

  /* File definitions */
  DEF_CONST(mIl, "IL", "FILE_OVERWRITE", IL_FILE_OVERWRITE);
  DEF_CONST(mIl, "IL", "FILE_MODE", IL_FILE_MODE);

  /* Palette definitions */
  DEF_CONST(mIl, "IL", "CONV_PAL", IL_CONV_PAL);

  /* Load fail definitions */
  DEF_CONST(mIl, "IL", "DEFAULT_ON_FAIL", IL_DEFAULT_ON_FAIL);

  /* Key colour definitions */
  DEF_CONST(mIl, "IL", "USE_KEY_COLOUR", IL_USE_KEY_COLOUR);
  DEF_CONST(mIl, "IL", "USE_KEY_COLOR", IL_USE_KEY_COLOR);

  /* Interlace definitions */
  DEF_CONST(mIl, "IL", "SAVE_INTERLACED", IL_SAVE_INTERLACED);
  DEF_CONST(mIl, "IL", "INTERLACE_MODE", IL_INTERLACE_MODE);

  /* Quantization definitions */
  DEF_CONST(mIl, "IL", "QUANTIZATION_MODE", IL_QUANTIZATION_MODE);
  DEF_CONST(mIl, "IL", "WU_QUANT", IL_WU_QUANT);
  DEF_CONST(mIl, "IL", "NEU_QUANT", IL_NEU_QUANT);
  DEF_CONST(mIl, "IL", "NEU_QUANT_SAMPLE", IL_NEU_QUANT_SAMPLE);

  /* Hints */
  DEF_CONST(mIl, "IL", "FASTEST", IL_FASTEST);
  DEF_CONST(mIl, "IL", "LESS_MEM", IL_LESS_MEM);
  DEF_CONST(mIl, "IL", "DONT_CARE", IL_DONT_CARE);
  DEF_CONST(mIl, "IL", "MEM_SPEED_HINT", IL_MEM_SPEED_HINT);
  DEF_CONST(mIl, "IL", "USE_COMPRESSION", IL_USE_COMPRESSION);
  DEF_CONST(mIl, "IL", "NO_COMPRESSION", IL_NO_COMPRESSION);
  DEF_CONST(mIl, "IL", "COMPRESSION_HINT", IL_COMPRESSION_HINT);
  
  /* Subimage types */
  DEF_CONST(mIl, "IL", "SUB_NEXT", IL_SUB_NEXT);
  DEF_CONST(mIl, "IL", "SUB_MIPMAP", IL_SUB_MIPMAP);
  DEF_CONST(mIl, "IL", "SUB_LAYER", IL_SUB_LAYER);

  /* Compression definitions */
  DEF_CONST(mIl, "IL", "COMPRESS_MODE", IL_COMPRESS_MODE);
  DEF_CONST(mIl, "IL", "COMPRESS_NONE", IL_COMPRESS_NONE);
  DEF_CONST(mIl, "IL", "COMPRESS_RLE", IL_COMPRESS_RLE);
  DEF_CONST(mIl, "IL", "COMPRESS_LZO", IL_COMPRESS_LZO);
  DEF_CONST(mIl, "IL", "COMPRESS_ZLIB", IL_COMPRESS_ZLIB);

  /* File format-specific values */
  DEF_CONST(mIl, "IL", "TGA_CREATE_STAMP", IL_TGA_CREATE_STAMP);
  DEF_CONST(mIl, "IL", "JPG_QUALITY", IL_JPG_QUALITY);
  DEF_CONST(mIl, "IL", "PNG_INTERLACE", IL_PNG_INTERLACE);
  DEF_CONST(mIl, "IL", "TGA_RLE", IL_TGA_RLE);
  DEF_CONST(mIl, "IL", "BMP_RLE", IL_BMP_RLE);
  DEF_CONST(mIl, "IL", "SGI_RLE", IL_SGI_RLE);
  DEF_CONST(mIl, "IL", "TGA_ID_STRING", IL_TGA_ID_STRING);
  DEF_CONST(mIl, "IL", "TGA_AUTHNAME_STRING", IL_TGA_AUTHNAME_STRING);
  DEF_CONST(mIl, "IL", "TGA_AUTHCOMMENT_STRING", IL_TGA_AUTHCOMMENT_STRING);
  DEF_CONST(mIl, "IL", "PNG_AUTHNAME_STRING", IL_PNG_AUTHNAME_STRING);
  DEF_CONST(mIl, "IL", "PNG_TITLE_STRING", IL_PNG_TITLE_STRING);
  DEF_CONST(mIl, "IL", "PNG_DESCRIPTION_STRING", IL_PNG_DESCRIPTION_STRING);
  DEF_CONST(mIl, "IL", "TIF_DESCRIPTION_STRING", IL_TIF_DESCRIPTION_STRING);
  DEF_CONST(mIl, "IL", "TIF_HOSTCOMPUTER_STRING", IL_TIF_HOSTCOMPUTER_STRING);
  DEF_CONST(mIl, "IL", "TIF_DOCUMENTNAME_STRING", IL_TIF_DOCUMENTNAME_STRING);
  DEF_CONST(mIl, "IL", "TIF_AUTHNAME_STRING", IL_TIF_AUTHNAME_STRING);
  DEF_CONST(mIl, "IL", "JPG_SAVE_FORMAT", IL_JPG_SAVE_FORMAT);
  DEF_CONST(mIl, "IL", "CHEAD_HEADER_STRING", IL_CHEAD_HEADER_STRING);
  DEF_CONST(mIl, "IL", "PCD_PICNUM", IL_PCD_PICNUM);

  /* DXTC definitions */
  DEF_CONST(mIl, "IL", "DXTC_FORMAT", IL_DXTC_FORMAT);
  DEF_CONST(mIl, "IL", "DXT1", IL_DXT1);
  DEF_CONST(mIl, "IL", "DXT2", IL_DXT2);
  DEF_CONST(mIl, "IL", "DXT3", IL_DXT3);
  DEF_CONST(mIl, "IL", "DXT4", IL_DXT4);
  DEF_CONST(mIl, "IL", "DXT5", IL_DXT5);
  DEF_CONST(mIl, "IL", "DXT_NO_COMP", IL_DXT_NO_COMP);
  DEF_CONST(mIl, "IL", "KEEP_DXTC_DATA", IL_KEEP_DXTC_DATA);
  DEF_CONST(mIl, "IL", "DXTC_DATA_FORMAT", IL_DXTC_DATA_FORMAT);

  /* Cube map definitions */
  DEF_CONST(mIl, "IL", "CUBEMAP_POSITIVEX", IL_CUBEMAP_POSITIVEX);
  DEF_CONST(mIl, "IL", "CUBEMAP_NEGATIVEX", IL_CUBEMAP_NEGATIVEX);
  DEF_CONST(mIl, "IL", "CUBEMAP_POSITIVEY", IL_CUBEMAP_POSITIVEY);
  DEF_CONST(mIl, "IL", "CUBEMAP_NEGATIVEY", IL_CUBEMAP_NEGATIVEY);
  DEF_CONST(mIl, "IL", "CUBEMAP_POSITIVEZ", IL_CUBEMAP_POSITIVEZ);
  DEF_CONST(mIl, "IL", "CUBEMAP_NEGATIVEZ", IL_CUBEMAP_NEGATIVEZ);

  /* Values */
  DEF_CONST(mIl, "IL", "VERSION_NUM", IL_VERSION_NUM);
  DEF_CONST(mIl, "IL", "IMAGE_WIDTH", IL_IMAGE_WIDTH);
  DEF_CONST(mIl, "IL", "IMAGE_HEIGHT", IL_IMAGE_HEIGHT);
  DEF_CONST(mIl, "IL", "IMAGE_DEPTH", IL_IMAGE_DEPTH);
  DEF_CONST(mIl, "IL", "IMAGE_SIZE_OF_DATA", IL_IMAGE_SIZE_OF_DATA);
  DEF_CONST(mIl, "IL", "IMAGE_BPP", IL_IMAGE_BPP);
  DEF_CONST(mIl, "IL", "IMAGE_BYTES_PER_PIXEL", IL_IMAGE_BYTES_PER_PIXEL);
  DEF_CONST(mIl, "IL", "IMAGE_BITS_PER_PIXEL", IL_IMAGE_BITS_PER_PIXEL);
  DEF_CONST(mIl, "IL", "IMAGE_FORMAT", IL_IMAGE_FORMAT);
  DEF_CONST(mIl, "IL", "IMAGE_TYPE", IL_IMAGE_TYPE);
  DEF_CONST(mIl, "IL", "PALETTE_TYPE", IL_PALETTE_TYPE);
  DEF_CONST(mIl, "IL", "PALETTE_SIZE", IL_PALETTE_SIZE);
  DEF_CONST(mIl, "IL", "PALETTE_BPP", IL_PALETTE_BPP);
  DEF_CONST(mIl, "IL", "PALETTE_NUM_COLS", IL_PALETTE_NUM_COLS);
  DEF_CONST(mIl, "IL", "PALETTE_BASE_TYPE", IL_PALETTE_BASE_TYPE);
  DEF_CONST(mIl, "IL", "NUM_IMAGES", IL_NUM_IMAGES);
  DEF_CONST(mIl, "IL", "NUM_MIPMAPS", IL_NUM_MIPMAPS);
  DEF_CONST(mIl, "IL", "NUM_LAYERS", IL_NUM_LAYERS);
  DEF_CONST(mIl, "IL", "ACTIVE_IMAGE", IL_ACTIVE_IMAGE);
  DEF_CONST(mIl, "IL", "ACTIVE_MIPMAP", IL_ACTIVE_MIPMAP);
  DEF_CONST(mIl, "IL", "ACTIVE_LAYER", IL_ACTIVE_LAYER);
  DEF_CONST(mIl, "IL", "CUR_IMAGE", IL_CUR_IMAGE);
  DEF_CONST(mIl, "IL", "IMAGE_DURATION", IL_IMAGE_DURATION);
  DEF_CONST(mIl, "IL", "IMAGE_PLANESIZE", IL_IMAGE_PLANESIZE);
  DEF_CONST(mIl, "IL", "IMAGE_BPC", IL_IMAGE_BPC);
  DEF_CONST(mIl, "IL", "IMAGE_OFFX", IL_IMAGE_OFFX);
  DEF_CONST(mIl, "IL", "IMAGE_OFFY", IL_IMAGE_OFFY);
  DEF_CONST(mIl, "IL", "IMAGE_CUBEFLAGS", IL_IMAGE_CUBEFLAGS);

  /* ILU constants */
  DEF_CONST(mIlu, "ILU", "FILTER", ILU_FILTER);
  DEF_CONST(mIlu, "ILU", "NEAREST", ILU_NEAREST);
  DEF_CONST(mIlu, "ILU", "LINEAR", ILU_LINEAR);
  DEF_CONST(mIlu, "ILU", "BILINEAR", ILU_BILINEAR);
  DEF_CONST(mIlu, "ILU", "SCALE_BOX", ILU_SCALE_BOX);
  DEF_CONST(mIlu, "ILU", "SCALE_TRIANGLE", ILU_SCALE_TRIANGLE);
  DEF_CONST(mIlu, "ILU", "SCALE_BELL", ILU_SCALE_BELL);
  DEF_CONST(mIlu, "ILU", "SCALE_BSPLINE", ILU_SCALE_BSPLINE);
  DEF_CONST(mIlu, "ILU", "SCALE_LANCZOS3", ILU_SCALE_LANCZOS3);
  DEF_CONST(mIlu, "ILU", "SCALE_MITCHELL", ILU_SCALE_MITCHELL);

  /* ILU Values */
  DEF_CONST(mIlu, "ILU", "PLACEMENT", ILU_PLACEMENT);
  DEF_CONST(mIlu, "ILU", "LOWER_LEFT", ILU_LOWER_LEFT);
  DEF_CONST(mIlu, "ILU", "LOWER_RIGHT", ILU_LOWER_RIGHT);
  DEF_CONST(mIlu, "ILU", "UPPER_LEFT", ILU_UPPER_LEFT);
  DEF_CONST(mIlu, "ILU", "UPPER_RIGHT", ILU_UPPER_RIGHT);
  DEF_CONST(mIlu, "ILU", "CENTER", ILU_CENTER);
  DEF_CONST(mIlu, "ILU", "CONVOLUTION_MATRIX", ILU_CONVOLUTION_MATRIX);
  DEF_CONST(mIlu, "ILU", "VERSION_NUM", ILU_VERSION_NUM);
}

void Init_devil(void) {
  mDevil = rb_define_module("DevIL");
  rb_define_const(mDevil, "DEVIL_VERSION", rb_str_new2(DEVIL_VERSION));

  mIl  = rb_define_module_under(mDevil, "IL");
  mIlu = rb_define_module_under(mDevil, "ILU");

  define_constants();

  /* IL methods */
  rb_define_method(mIl, "active_image", il_active_im, 1);
  rb_define_method(mIl, "ActiveImage", il_active_im, 1);
  rb_define_method(mIl, "active_layer", il_active_layer, 1);
  rb_define_method(mIl, "ActiveLayer", il_active_layer, 1);
  rb_define_method(mIl, "active_mipmap", il_active_mipmap, 1);
  rb_define_method(mIl, "ActiveMipmap", il_active_mipmap, 1);
  rb_define_method(mIl, "apply_pal", il_apply_pal, 1);
  rb_define_method(mIl, "ApplyPal", il_apply_pal, 1);
  rb_define_method(mIl, "apply_profile", il_apply_profile, 2);
  rb_define_method(mIl, "ApplyProfile", il_apply_profile, 2);
  rb_define_method(mIl, "bind_image", il_bind_im, 1);
  rb_define_method(mIl, "BindImage", il_bind_im, 1);
  rb_define_method(mIl, "blit", il_blit, 10);
  rb_define_method(mIl, "Blit", il_blit, 10);
  rb_define_method(mIl, "clear_color", il_clear_color, 4);
  rb_define_method(mIl, "ClearColor", il_clear_color, 4);
  rb_define_method(mIl, "clear_colour", il_clear_color, 4);
  rb_define_method(mIl, "ClearColour", il_clear_color, 4);
  rb_define_method(mIl, "clear_image", il_clear_im, 0);
  rb_define_method(mIl, "ClearImage", il_clear_im, 0);
  rb_define_method(mIl, "clone_cur_image", il_clone_cur_im, 0);
  rb_define_method(mIl, "CloneCurImage", il_clone_cur_im, 0);
  rb_define_method(mIl, "compress_func", il_compress_func, 1);
  rb_define_method(mIl, "CompressFunc", il_compress_func, 1);
  rb_define_method(mIl, "convert_image", il_convert_im, 2);
  rb_define_method(mIl, "ConvertImage", il_convert_im, 2);
  rb_define_method(mIl, "convert_pal", il_convert_pal, 1);
  rb_define_method(mIl, "ConvertPal", il_convert_pal, 1);
  rb_define_method(mIl, "copy_image", il_copy_im, 1);
  rb_define_method(mIl, "CopyImage", il_copy_im, 1);
  rb_define_method(mIl, "copy_pixels", il_copy_pixels, 9);
  rb_define_method(mIl, "CopyPixels", il_copy_pixels, 9);
  rb_define_method(mIl, "create_sub_image", il_create_sub_im, 2);
  rb_define_method(mIl, "CreateSubImage", il_create_sub_im, 2);
  rb_define_method(mIl, "default_image", il_default_im, 0);
  rb_define_method(mIl, "DefaultImage", il_default_im, 0);
  rb_define_method(mIl, "delete_images", il_delete_ims, -1);
  rb_define_method(mIl, "DeleteImages", il_delete_ims, -1);
  rb_define_method(mIl, "disable", il_disable, 1);
  rb_define_method(mIl, "Disable", il_disable, 1);
  rb_define_method(mIl, "enable", il_enable, 1);
  rb_define_method(mIl, "Enable", il_enable, 1);
  rb_define_method(mIl, "format_func", il_format_func, 1);
  rb_define_method(mIl, "FormatFunc", il_format_func, 1);
  rb_define_method(mIl, "gen_images", il_gen_ims, -1);
  rb_define_method(mIl, "GenImages", il_gen_ims, -1);
  rb_define_method(mIl, "get_alpha", il_get_alpha, 1);
  rb_define_method(mIl, "GetAlpha", il_get_alpha, 1);
  rb_define_method(mIl, "get_boolean", il_get_bool, 1);
  rb_define_method(mIl, "GetBoolean", il_get_bool, 1);
  rb_define_method(mIl, "get_data", il_get_data, 0);
  rb_define_method(mIl, "GetData", il_get_data, 0);
  rb_define_method(mIl, "get_dxtc_data", il_get_dxtc_data, 2);
  rb_define_method(mIl, "GetDXTCData", il_get_dxtc_data, 2);
  rb_define_method(mIl, "get_error", il_get_err, 0);
  rb_define_method(mIl, "GetError", il_get_err, 0);
  rb_define_method(mIl, "get_integer", il_get_int, 1);
  rb_define_method(mIl, "GetInteger", il_get_int, 1);
  rb_define_method(mIl, "get_lump_pos", il_get_lump_pos, 0);
  rb_define_method(mIl, "GetLumpPos", il_get_lump_pos, 0);
  rb_define_method(mIl, "get_palette", il_get_palette, 0);
  rb_define_method(mIl, "GetPalette", il_get_palette, 0);
  rb_define_method(mIl, "get_string", il_get_string, 1);
  rb_define_method(mIl, "GetString", il_get_string, 1);
  rb_define_method(mIl, "hint", il_hint, 2);
  rb_define_method(mIl, "Hint", il_hint, 2);
  rb_define_method(mIl, "is_disabled", il_is_disabled, 1);
  rb_define_method(mIl, "IsDisabled", il_is_disabled, 1);
  rb_define_method(mIl, "is_disabled?", il_is_disabled, 1);
  rb_define_method(mIl, "IsDisabled?", il_is_disabled, 1);
  rb_define_method(mIl, "is_enabled", il_is_enabled, 1);
  rb_define_method(mIl, "IsEnabled", il_is_enabled, 1);
  rb_define_method(mIl, "is_enabled?", il_is_enabled, 1);
  rb_define_method(mIl, "IsEnabled?", il_is_enabled, 1);
  rb_define_method(mIl, "is_image", il_is_im, 1);
  rb_define_method(mIl, "IsImage", il_is_im, 1);
  rb_define_method(mIl, "is_image?", il_is_im, 1);
  rb_define_method(mIl, "IsImage?", il_is_im, 1);
  rb_define_method(mIl, "is_valid", il_is_valid, 2);
  rb_define_method(mIl, "IsValid", il_is_valid, 2);
  rb_define_method(mIl, "is_valid?", il_is_valid, 2);
  rb_define_method(mIl, "IsValid?", il_is_valid, 2);
  rb_define_method(mIl, "is_valid_f", il_is_valid_f, 2);
  rb_define_method(mIl, "IsValidF", il_is_valid_f, 2);
  rb_define_method(mIl, "is_valid_f?", il_is_valid_f, 2);
  rb_define_method(mIl, "IsValidF?", il_is_valid_f, 2);
  rb_define_method(mIl, "is_valid_l", il_is_valid_l, 2);
  rb_define_method(mIl, "IsValidL", il_is_valid_l, 2);
  rb_define_method(mIl, "is_valid_l?", il_is_valid_l, 2);
  rb_define_method(mIl, "IsValidL?", il_is_valid_l, 2);
  rb_define_method(mIl, "key_color", il_key_color, 4);
  rb_define_method(mIl, "KeyColor", il_key_color, 4);
  rb_define_method(mIl, "key_colour", il_key_color, 4);
  rb_define_method(mIl, "KeyColour", il_key_color, 4);
  rb_define_method(mIl, "load", il_load, 2);
  rb_define_method(mIl, "Load", il_load, 2);
  rb_define_method(mIl, "load_f", il_load_f, 2);
  rb_define_method(mIl, "LoadF", il_load_f, 2);
  rb_define_method(mIl, "load_image", il_load_im, 1);
  rb_define_method(mIl, "LoadImage", il_load_im, 1);
  rb_define_method(mIl, "load_l", il_load_l, 2);
  rb_define_method(mIl, "LoadL", il_load_l, 2);
  rb_define_method(mIl, "load_pal", il_load_pal, 1);
  rb_define_method(mIl, "LoadPal", il_load_pal, 1);
  rb_define_method(mIl, "origin_func", il_origin_func, 1);
  rb_define_method(mIl, "OriginFunc", il_origin_func, 1);
  rb_define_method(mIl, "overlay_image", il_overlay_im, 4);
  rb_define_method(mIl, "OverlayImage", il_overlay_im, 4);
  rb_define_method(mIl, "pop_attrib", il_pop_attrib, 0);
  rb_define_method(mIl, "PopAttrib", il_pop_attrib, 0);
  rb_define_method(mIl, "push_attrib", il_push_attrib, 1);
  rb_define_method(mIl, "PushAttrib", il_push_attrib, 1);
  rb_define_method(mIl, "register_format", il_register_format, 1);
  rb_define_method(mIl, "ResisterFormat", il_register_format, 1);
  rb_define_method(mIl, "register_load", il_register_load, 2);
  rb_define_method(mIl, "ResisterLoad", il_register_load, 2);
  rb_define_method(mIl, "register_mipnum", il_register_mipnum, 1);
  rb_define_method(mIl, "ResisterMipnum", il_register_mipnum, 1);
  rb_define_method(mIl, "register_num_images", il_register_num_ims, 1);
  rb_define_method(mIl, "ResisterNumImages", il_register_num_ims, 1);
  rb_define_method(mIl, "register_origin", il_register_origin, 1);
  rb_define_method(mIl, "ResisterOrigin", il_register_origin, 1);
  rb_define_method(mIl, "register_pal", il_register_pal, 1);
  rb_define_method(mIl, "ResisterPal", il_register_pal, 1);
  rb_define_method(mIl, "register_save", il_register_save, 2);
  rb_define_method(mIl, "ResisterSave", il_register_save, 2);
  rb_define_method(mIl, "register_type", il_register_type, 1);
  rb_define_method(mIl, "ResisterType", il_register_type, 1);
  rb_define_method(mIl, "remove_load", il_remove_load, 1);
  rb_define_method(mIl, "RemoveLoad", il_remove_load, 1);
  rb_define_method(mIl, "remove_save", il_remove_save, 1);
  rb_define_method(mIl, "RemoveSave", il_remove_save, 1);
  rb_define_method(mIl, "reset_memory", il_reset_mem, 0);
  rb_define_method(mIl, "ResetMemory", il_reset_mem, 0);
  rb_define_method(mIl, "reset_read", il_reset_read, 0);
  rb_define_method(mIl, "ResetRead", il_reset_read, 0);
  rb_define_method(mIl, "reset_write", il_reset_write, 0);
  rb_define_method(mIl, "ResetWrite", il_reset_write, 0);
  rb_define_method(mIl, "save", il_save, 2);
  rb_define_method(mIl, "Save", il_save, 2);
  rb_define_method(mIl, "save_f", il_save_f, 2);
  rb_define_method(mIl, "SaveF", il_save_f, 2);
  rb_define_method(mIl, "save_image", il_save_im, 1);
  rb_define_method(mIl, "SaveImage", il_save_im, 1);
  rb_define_method(mIl, "save_l", il_save_l, 2);
  rb_define_method(mIl, "SaveL", il_save_l, 2);
  rb_define_method(mIl, "save_pal", il_save_pal, 1);
  rb_define_method(mIl, "SavePal", il_save_pal, 1);
  rb_define_method(mIl, "set_data", il_set_data, 1);
  rb_define_method(mIl, "SetData", il_set_data, 1);
  rb_define_method(mIl, "set_duration", il_set_duration, 1);
  rb_define_method(mIl, "SetDuration", il_set_duration, 1);
  rb_define_method(mIl, "set_integer", il_set_int, 2);
  rb_define_method(mIl, "SetInteger", il_set_int, 2);
  rb_define_method(mIl, "set_memory", il_set_mem, 1);
  rb_define_method(mIl, "SetMemory", il_set_mem, 1);
  rb_define_method(mIl, "set_pixels", il_set_pixels, 9);
  rb_define_method(mIl, "SetPixels", il_set_pixels, 9);
  rb_define_method(mIl, "set_string", il_set_string, 2);
  rb_define_method(mIl, "SetString", il_set_string, 2);
  rb_define_method(mIl, "set_write", il_set_write, 6);
  rb_define_method(mIl, "SetWrite", il_set_write, 6);
  rb_define_method(mIl, "shutdown", il_shutdown, 0);
  rb_define_method(mIl, "Shutdown", il_shutdown, 0);
  rb_define_method(mIl, "tex_image", il_tex_im, 7);
  rb_define_method(mIl, "TexImage", il_tex_im, 7);
  rb_define_method(mIl, "type_func", il_type_func, 1);
  rb_define_method(mIl, "TypeFunc", il_type_func, 1);

  rb_define_method(mIl, "load_data", il_load_data, 5);
  rb_define_method(mIl, "LoadData", il_load_data, 5);
  rb_define_method(mIl, "load_data_f", il_load_data_f, 5);
  rb_define_method(mIl, "LoadDataF", il_load_data_f, 5);
  rb_define_method(mIl, "load_data_l", il_load_data_l, 6);
  rb_define_method(mIl, "LoadDataL", il_load_data_l, 6);
  rb_define_method(mIl, "save_data", il_save_data, 1);
  rb_define_method(mIl, "SaveData", il_save_data, 1);

  /* rb_define_method(mDevil, "load_from_jpeg_struct", il_load_from_jpeg_struct, 1);
  rb_define_method(mDevil, "LoadFromJpegStruct", il_load_from_jpeg_struct, 1);
  rb_define_method(mDevil, "save_from_jpeg_struct", il_save_from_jpeg_struct, 1);
  rb_define_method(mDevil, "SaveFromJpegStruct", il_save_from_jpeg_struct, 1); */


  /***************/
  /* ILU methods */
  /***************/
  rb_define_method(mIlu, "alienify", ilu_alienify, 0);
  rb_define_method(mIlu, "Alienify", ilu_alienify, 0);
  rb_define_method(mIlu, "blur_avg", ilu_blur_avg, 1);
  rb_define_method(mIlu, "BlurAvg", ilu_blur_avg, 1);
  rb_define_method(mIlu, "blur_gaussian", ilu_blur_gaussian, 1);
  rb_define_method(mIlu, "BlurGaussian", ilu_blur_gaussian, 1);
  rb_define_method(mIlu, "build_mipmaps", ilu_build_mipmaps, 0);
  rb_define_method(mIlu, "BuildMipmaps", ilu_build_mipmaps, 0);
  rb_define_method(mIlu, "colors_used", ilu_colors_used, 0);
  rb_define_method(mIlu, "ColorsUsed", ilu_colors_used, 0);
  rb_define_method(mIlu, "colours_used", ilu_colors_used, 0);
  rb_define_method(mIlu, "ColoursUsed", ilu_colors_used, 0);
  rb_define_method(mIlu, "compare_image", ilu_compare_im, 1);
  rb_define_method(mIlu, "CompareImage", ilu_compare_im, 1);
  rb_define_method(mIlu, "contrast", ilu_contrast, 1);
  rb_define_method(mIlu, "Contrast", ilu_contrast, 1);
  rb_define_method(mIlu, "crop", ilu_crop, 6);
  rb_define_method(mIlu, "Crop", ilu_crop, 6);
  rb_define_method(mIlu, "delete_image", ilu_delete_im, 1);
  rb_define_method(mIlu, "DeleteImage", ilu_delete_im, 1);
  rb_define_method(mIlu, "edge_detect_e", ilu_edge_detect_e, 1);
  rb_define_method(mIlu, "EdgeDetectE", ilu_edge_detect_e, 1);
  rb_define_method(mIlu, "edge_detect_p", ilu_edge_detect_p, 1);
  rb_define_method(mIlu, "EdgeDetectP", ilu_edge_detect_p, 1);
  rb_define_method(mIlu, "edge_detect_s", ilu_edge_detect_s, 1);
  rb_define_method(mIlu, "EdgeDetectS", ilu_edge_detect_s, 1);
  rb_define_method(mIlu, "emboss", ilu_emboss, 0);
  rb_define_method(mIlu, "Emboss", ilu_emboss, 0);
  rb_define_method(mIlu, "enlarge_canvas", ilu_enlarge_canvas, 3);
  rb_define_method(mIlu, "EnlargeCanvas", ilu_enlarge_canvas, 3);
  rb_define_method(mIlu, "enlarge_image", ilu_enlarge_im, 3);
  rb_define_method(mIlu, "EnlargeImage", ilu_enlarge_im, 3);
  rb_define_method(mIlu, "equalize", ilu_equalize, 0);
  rb_define_method(mIlu, "Equalize", ilu_equalize, 0);
  rb_define_method(mIlu, "error_string", ilu_error_string, 1);
  rb_define_method(mIlu, "ErrorString", ilu_error_string, 1);
  rb_define_method(mIlu, "flip_image", ilu_flip_im, 0);
  rb_define_method(mIlu, "FlipImage", ilu_flip_im, 0);
  rb_define_method(mIlu, "gamma_correct", ilu_gamma_correct, 1);
  rb_define_method(mIlu, "GammaCorrect", ilu_gamma_correct, 1);
  rb_define_method(mIlu, "gen_image", ilu_gen_im, 0);
  rb_define_method(mIlu, "GenImage", ilu_gen_im, 0);
  rb_define_method(mIlu, "get_image_info", ilu_get_im_info, 0);
  rb_define_method(mIlu, "GetImageInfo", ilu_get_im_info, 0);
  rb_define_method(mIlu, "get_integer", ilu_get_int, 1);
  rb_define_method(mIlu, "GetInteger", ilu_get_int, 1);
  rb_define_method(mIlu, "get_string", ilu_get_string, 1);
  rb_define_method(mIlu, "GetString", ilu_get_string, 1);
  rb_define_method(mIlu, "image_parameter", ilu_im_parameter, 2);
  rb_define_method(mIlu, "ImageParameter", ilu_im_parameter, 2);
  rb_define_method(mIlu, "invert_alpha", ilu_invert_alpha, 0);
  rb_define_method(mIlu, "InvertAlpha", ilu_invert_alpha, 0);
  rb_define_method(mIlu, "load_image", ilu_load_im, 1);
  rb_define_method(mIlu, "LoadImage", ilu_load_im, 1);
  rb_define_method(mIlu, "mirror", ilu_mirror, 0);
  rb_define_method(mIlu, "Mirror", ilu_mirror, 0);
  rb_define_method(mIlu, "negative", ilu_negative, 0);
  rb_define_method(mIlu, "Negative", ilu_negative, 0);
  rb_define_method(mIlu, "noisify", ilu_noisify, 1);
  rb_define_method(mIlu, "Noisify", ilu_noisify, 1);
  rb_define_method(mIlu, "pixelize", ilu_pixelize, 1);
  rb_define_method(mIlu, "Pixelize", ilu_pixelize, 1);
  rb_define_method(mIlu, "region_fv", ilu_region_fv, 1);
  rb_define_method(mIlu, "Regionfv", ilu_region_fv, 1);
  rb_define_method(mIlu, "region_iv", ilu_region_iv, 1);
  rb_define_method(mIlu, "Regioniv", ilu_region_iv, 1);
  rb_define_method(mIlu, "replace_color", ilu_replace_color, 4);
  rb_define_method(mIlu, "ReplaceColor", ilu_replace_color, 4);
  rb_define_method(mIlu, "replace_colour", ilu_replace_color, 4);
  rb_define_method(mIlu, "ReplaceColour", ilu_replace_color, 4);
  rb_define_method(mIlu, "rotate", ilu_rotate, 1);
  rb_define_method(mIlu, "Rotate", ilu_rotate, 1);
  rb_define_method(mIlu, "rotate_3d", ilu_rotate_3d, 4);
  rb_define_method(mIlu, "Rotate3D", ilu_rotate_3d, 4);
  rb_define_method(mIlu, "saturate_1f", ilu_saturate_1f, 1);
  rb_define_method(mIlu, "Saturate1f", ilu_saturate_1f, 1);
  rb_define_method(mIlu, "saturate_4f", ilu_saturate_4f, 4);
  rb_define_method(mIlu, "Saturate4f", ilu_saturate_4f, 4);
  rb_define_method(mIlu, "scale", ilu_scale, 3);
  rb_define_method(mIlu, "Scale", ilu_scale, 3);
  rb_define_method(mIlu, "scale_colors", ilu_scale_colors, 3);
  rb_define_method(mIlu, "ScaleColors", ilu_scale_colors, 3);
  rb_define_method(mIlu, "scale_colours", ilu_scale_colors, 3);
  rb_define_method(mIlu, "ScaleColours", ilu_scale_colors, 3);
  rb_define_method(mIlu, "swap_colors", ilu_swap_colors, 0);
  rb_define_method(mIlu, "SwapColors", ilu_swap_colors, 0);
  rb_define_method(mIlu, "swap_colours", ilu_swap_colors, 0);
  rb_define_method(mIlu, "SwapColours", ilu_swap_colors, 0);
  rb_define_method(mIlu, "wave", ilu_wave, 1);
  rb_define_method(mIlu, "Wave", ilu_wave, 1);


  /***********************/
  /* initialize IL & ILU */
  /***********************/
  ilInit();
  iluInit();

  load_procs = rb_hash_new();
  save_procs = rb_hash_new();
}

/*********************/
/* UTILITY FUNCTIONS */
/*********************/
static char *get_ext(char *str) {
  char *ext;
  int len;

  if (str && (len = strlen(str)) > 1)
    for (ext = str + len; ext != str; ext--)
      if (*ext == '.')
        return strdup(ext + 1);

  return NULL;
}

