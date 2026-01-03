#include <obs-module.h>
#include <plugin-support.h>
#include <graphics/effect.h>

#include <cstdint>
#include <new>

class TintFilter final {
public:
	TintFilter(obs_data_t *settings, obs_source_t *source) : context_(source)
	{
		obs_enter_graphics();

		char *path = obs_module_file("effects/tint.effect");
		if (path) {
			effect_ = gs_effect_create_from_file(path, nullptr);
			bfree(path);
		} else {
			obs_log(LOG_WARNING, "[tint_filter] obs_module_file() failed: effects/tint.effect");
		}

		if (effect_) {
			param_tint_color_ = gs_effect_get_param_by_name(effect_, "tint_color");
			param_strength_ = gs_effect_get_param_by_name(effect_, "strength");
		} else {
			obs_log(LOG_WARNING, "[tint_filter] Failed to load effect: effects/tint.effect");
		}

		obs_leave_graphics();

		Update(settings);
	}

	~TintFilter()
	{
		obs_enter_graphics();
		if (effect_) {
			gs_effect_destroy(effect_);
			effect_ = nullptr;
		}
		obs_leave_graphics();
	}

	void Update(obs_data_t *settings)
	{
		const std::uint32_t rgba = static_cast<std::uint32_t>(obs_data_get_int(settings, "tint_color"));
		vec4_from_rgba(&tint_color_, rgba);

		strength_ = static_cast<float>(obs_data_get_double(settings, "strength"));
	}

	void Render()
	{
		if (!effect_) {
			obs_source_skip_video_filter(context_);
			return;
		}

		if (!obs_source_process_filter_begin(context_, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
			return;
		}

		gs_effect_set_vec4(param_tint_color_, &tint_color_);
		gs_effect_set_float(param_strength_, strength_);

		obs_source_process_filter_end(context_, effect_, 0, 0);
	}

	static const char *GetName(void *unused)
	{
		UNUSED_PARAMETER(unused);
		return obs_module_text("TintFilterName");
	}

	static void *Create(obs_data_t *settings, obs_source_t *source)
	{
		return new (std::nothrow) TintFilter(settings, source);
	}

	static void Destroy(void *data) { delete static_cast<TintFilter *>(data); }

	static void UpdateCallback(void *data, obs_data_t *settings)
	{
		static_cast<TintFilter *>(data)->Update(settings);
	}

	static obs_properties_t *Properties(void *unused)
	{
		UNUSED_PARAMETER(unused);

		obs_properties_t *props = obs_properties_create();
		obs_properties_add_color(props, "tint_color", obs_module_text("TintColor"));
		obs_properties_add_float_slider(props, "strength", obs_module_text("Strength"), 0.0, 1.0, 0.01);
		return props;
	}

	static void VideoRender(void *data, gs_effect_t *unused)
	{
		UNUSED_PARAMETER(unused);
		static_cast<TintFilter *>(data)->Render();
	}

private:
	obs_source_t *context_ = nullptr;

	gs_effect_t *effect_ = nullptr;
	gs_eparam_t *param_tint_color_ = nullptr;
	gs_eparam_t *param_strength_ = nullptr;

	struct vec4 tint_color_ = {};
	float strength_ = 0.35f;
};

void register_tint_filter(void)
{
	static obs_source_info info = {};

	info.id = "tint_filter";
	info.type = OBS_SOURCE_TYPE_FILTER;
	info.output_flags = OBS_SOURCE_VIDEO;

	info.get_name = TintFilter::GetName;
	info.create = TintFilter::Create;
	info.destroy = TintFilter::Destroy;
	info.update = TintFilter::UpdateCallback;
	info.get_properties = TintFilter::Properties;
	info.video_render = TintFilter::VideoRender;

	obs_register_source(&info);
}
