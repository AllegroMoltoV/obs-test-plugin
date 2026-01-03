#include <obs-module.h>
#include <graphics/effect.h>

struct tint_filter_data {
	obs_source_t *context;

	gs_effect_t *effect;
	gs_eparam_t *param_tint_color;
	gs_eparam_t *param_strength;

	struct vec4 tint_color;
	float strength;
};

static const char *tint_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("TintFilterName");
}

static void tint_filter_update(void *data, obs_data_t *settings)
{
	struct tint_filter_data *f = data;

	uint32_t rgba = (uint32_t)obs_data_get_int(settings, "tint_color");
	vec4_from_rgba(&f->tint_color, rgba);

	f->strength = (float)obs_data_get_double(settings, "strength");
}

static obs_properties_t *tint_filter_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	obs_properties_add_color(props, "tint_color", obs_module_text("TintColor"));
	obs_properties_add_float_slider(props, "strength", obs_module_text("Strength"), 0.0, 1.0, 0.01);

	return props;
}

static void *tint_filter_create(obs_data_t *settings, obs_source_t *source)
{
	struct tint_filter_data *f = bzalloc(sizeof(*f));
	f->context = source;

	obs_enter_graphics();

	char *path = obs_module_file("effects/tint.effect");
	f->effect = gs_effect_create_from_file(path, NULL);
	bfree(path);

	if (f->effect) {
		f->param_tint_color = gs_effect_get_param_by_name(f->effect, "tint_color");
		f->param_strength = gs_effect_get_param_by_name(f->effect, "strength");
	}

	obs_leave_graphics();

	tint_filter_update(f, settings);
	return f;
}

static void tint_filter_destroy(void *data)
{
	struct tint_filter_data *f = data;

	obs_enter_graphics();
	if (f->effect) {
		gs_effect_destroy(f->effect);
	}
	obs_leave_graphics();

	bfree(f);
}

static void tint_filter_render(void *data, gs_effect_t *unused)
{
	UNUSED_PARAMETER(unused);
	struct tint_filter_data *f = data;

	if (!f->effect) {
		obs_source_skip_video_filter(f->context);
		return;
	}

	if (!obs_source_process_filter_begin(f->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
		return;
	}

	gs_effect_set_vec4(f->param_tint_color, &f->tint_color);
	gs_effect_set_float(f->param_strength, f->strength);

	obs_source_process_filter_end(f->context, f->effect, 0, 0);
}

static struct obs_source_info tint_filter_info = {
	.id = "tint_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = tint_filter_get_name,
	.create = tint_filter_create,
	.destroy = tint_filter_destroy,
	.update = tint_filter_update,
	.get_properties = tint_filter_properties,
	.video_render = tint_filter_render,
};

void register_tint_filter(void)
{
	obs_register_source(&tint_filter_info);
}