#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <filesystem>
#include <map>
#include <algorithm>

#include "SpiceUsr.h"

#include "chronoflux.hpp"

#include <mach-o/dyld.h>
#include <libgen.h>
#include <unistd.h>

void SetupMacOSBundlePath()
{
	char path[1024];
	uint32_t size = sizeof(path);
	if(_NSGetExecutablePath(path, &size) == 0)
	{
		char* exec_dir = dirname(path);
		
		std::string res_path = std::string(exec_dir) + "/../Resources";
		chdir(res_path.c_str());
	}
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct BodyConsts
{
	const char* name;
	const char* label;
	const char* frame;
	
	double period;
	double semi_major;

	double re;
	double rp;
};

std::vector<BodyConsts> bodies;

struct MoonData
{
	const char* name;
	const char* id;
	double pos_rel[3];
	float screen_pos[2];
	double z_depth;
	bool is_visible;
};

struct ObservationData
{
	bool use_refraction = false;

	chronoflux::TimePoint tp;
	double et;

	float tz_offsets[3] = {0.0f, 9.0f, -10.0f};
	const char* tz_names[3] = {"UT", "JST", "HST"};
	int edit_tz = 0;

	int target_index = 1; // 0: Mercury, 1: Venus, 2: Mars, 3: Jupiter

	double pos_p_sun[3], pos_e_sun[3];
	double dist_pe;
	double phase_angle;
	double illumination;
	double angular_size;
	double magnitude;

	double sun_dir_x;
	double sun_dir_y;
	double sun_dir_z;

	double sun_alt;
	double sun_az;
	double obj_alt;
	double obj_az;
	double moon_alt;
	double moon_az;
	double moon_age;
	double moon_illumination;
	double moon_pa;
	
	int tz_index = 1; 
	int site_index = 0;
	bool is_realtime = true;

	double elongation;
	double ra, dec;
	double np_angle;
	double airmass;
	double radial_vel;
	double ls_deg;
	double ssp_lon, ssp_lat;
	double sep_lon, sep_lat;

	ImVec2 lat_pts[5][100];
	ImVec2 lt_mer_pts[12][100];
	ImVec2 local_up_vec;

	ImVec2 lon_pts[12][100];
	ImVec2 lon_label_pts[12];

	bool show_latitude = true;   
	bool show_local_time = true;
	bool show_longitude = true;
	bool show_outline = true;

	std::string twilight_state;
	ImVec4 state_color;

	double view_x[3], view_y[3], view_z[3];

	MoonData galilean_moons[4] =
	{
		{"Io", "501"},
		{"Europa", "502"},
		{"Ganymede", "503"},
		{"Callisto", "504"}
	};

	bool show_moons = false;
	bool prev_lat = true;
	bool prev_lon = true;
	bool prev_lt = true;
	bool prev_ol = true;

	double m_p2j[3][3];
	double v_np_j2k[3];
	double sun_dir_j2k[3];
};

struct TZ
{
	const char* name;
	double offset_hours;
};

const TZ timezones[] =
{
	{"UTC", 0.0},
	{"JST", 9.0},
	{"HST", -10.0}
};

struct Site
{
	const char* name;
	double lat;
	double lon;
	double alt;
};

const Site sites[] =
{
	{"IRTF, Mauna Kea, Hawai", 19.82620, -155.47204, 4.200},
	{"T60 Observatory, Haleakala, Hawai", 20.70739, -156.25691, 3.000}, 
	{"NAOJ, Mitaka, Tokyo", 35.67654, 139.53791, 0.0567},
	{"Tohoku U., Sendai, Miyagi", 38.25739, 140.83630, 0.1565},
	{"R-CCS, Kobe, Hyogo", 34.65337, 135.22056, 0.0057},
	{"ALMA, Atacama, Chile", -23.029, -67.755, 5.060}, 
	{"Subaru Telescope, Mauna Kea, Hawai", 19.82542, -155.47607, 4.139}, 
	{"Sendai Observatory, Sendai, Miyagi", 38.25689976307643, 140.75534538428434, 0.1655}
};

double ApplyRefraction(double h, double site_alt = 0.0)
{
	double h_deg = h * dpr_c();
	
	if(h_deg < -0.575)
	{
		return h;
	}

	const double T0_sea = 288.15;
	const double P0_sea = 1013.25;
	
	double T_site = T0_sea - 0.0098 * site_alt;
	double P_site = P0_sea * pow(1.0 - 0.000022557 * site_alt, 5.2559);
	double f = (P_site / 1010.0) * (283.15 / T_site);
	double r_std = 1.02 / tan((h_deg + 10.3 / (h_deg + 5.11)) * rpd_c());

	return (h_deg + f * r_std / 60.0) * rpd_c();
}

void CalculateObservation(ObservationData& data)
{
	const BodyConsts& body = bodies[data.target_index];
	double lt;

	if(data.is_realtime)
	{
		data.tp = chronoflux::now(0.0);
	}

	std::string time_str = data.tp.format("%Y-%m-%d %H:%M:%S") + " UTC";
	double et_raw;
	str2et_c(time_str.c_str(), &et_raw);
	data.et = et_raw;

	double s_earth[3];
	spkpos_c(body.name, data.et, "ECLIPJ2000", "LT+S", "SUN", data.pos_p_sun, &lt);
	spkpos_c("EARTH",   data.et, "ECLIPJ2000", "LT+S", "SUN", data.pos_e_sun, &lt);

	double state[6];
	spkezr_c(body.name, data.et, "J2000", "LT+S", "EARTH", state, &lt);
	double pos_geo[3] = {state[0], state[1], state[2]};
	data.dist_pe = vnorm_c(pos_geo);
	recrad_c(pos_geo, &data.dist_pe, &data.ra, &data.dec);

	double pos[3] = {state[0], state[1], state[2]};
	double vel[3] = {state[3], state[4], state[5]};
	spkpos_c("SUN", data.et, "J2000", "LT+S", "EARTH", s_earth, &lt);
	data.elongation = vsep_c(pos, s_earth) * dpr_c();
	double unit_p[3];
	vhat_c(pos, unit_p);
	data.radial_vel = vdot_c(vel, unit_p);

	const Site& current_site = sites[data.site_index];
	double p_obs_fixed[3], re = 6378.137, rp = 6356.7523;
	georec_c(current_site.lon * rpd_c(), current_site.lat * rpd_c(), current_site.alt, re, (re-rp)/re, p_obs_fixed);
	double r_j2k_fixed[3][3], r_fixed_j2k[3][3];
	pxform_c("J2000", "ITRF93", data.et, r_j2k_fixed);
	invert_c(r_j2k_fixed, r_fixed_j2k);
	double z_up[3], y_west[3], x_north[3], z_axis[3] = {0,0,1};
	surfnm_c(re, re, rp, p_obs_fixed, z_up);
	vcrss_c(z_up, z_axis, y_west); vhat_c(y_west, y_west);
	vcrss_c(y_west, z_up, x_north); vhat_c(x_north, x_north);

	double r_au = vnorm_c(data.pos_p_sun) / 149597870.7;
	double d_au = data.dist_pe / 149597870.7;

	double phase_ang = phaseq_c(data.et, body.name, "SUN", "EARTH", "LT+S");
	double alpha = phase_ang * dpr_c();

	data.magnitude = 5.0 * std::log10(r_au * d_au);

	std::vector<double> c_mag;

	if(data.target_index == 0)
	{//水星
		c_mag = {-0.613, 6.3280E-2, -1.6336E-3, 3.3644E-5, -3.465E-7, 1.6893E-9, -3.30334E-12};

		for(int i = 0; i < c_mag.size(); ++i)
		{
			data.magnitude += c_mag[i] * std::pow(alpha, i);
		}
	}
	else if(data.target_index == 1)
	{//金星
		if(alpha <= 163.7)
		{
			c_mag = {-4.384, -1.044E-3, 3.687E-4, -2.814E-6, 8.938E-9};
		}
		else
		{
			c_mag = {236.05828, -2.81914, 8.39034E-3};
		}

		for(int i = 0; i < c_mag.size(); ++i)
		{
			data.magnitude += c_mag[i] * std::pow(alpha, i);
		}
	}
	else if(data.target_index == 2)
	{//火星
		if(alpha <= 50.0)
		{
			c_mag = {-1.601, 0.02267, -0.0001302};
		}
		else
		{
			c_mag = {-0.367, -0.02573, 0.0003445};
		}

		for(int i = 0; i < c_mag.size(); ++i)
		{
			data.magnitude += c_mag[i] * std::pow(alpha, i);
		}
	}
	else if(data.target_index == 3)
	{//木星
		if(alpha <= 12.0)
		{
			c_mag = {-9.395, -3.7E-4, 6.16E-4};

			for(int i = 0; i < c_mag.size(); ++i)
			{
				data.magnitude += c_mag[i] * std::pow(alpha, i);
			}
		}
		else
		{
			data.magnitude -= 9.428;

			c_mag = {1.0, -1.507, -0.363, -0.062, 2.809, -1.876};

			double temp = 0.0;

			for(int i = 0; i < c_mag.size(); ++i)
			{
				temp += c_mag[i] * std::pow(alpha / 180.0, i);
			}

			data.magnitude += -2.5 * std::log10(temp);
		}
	}
	else if(data.target_index == 4)
	{//土星 (Saturn)
		double ring_z[3] = {data.m_p2j[0][2], data.m_p2j[1][2], data.m_p2j[2][2]};
		
		double sin_beta_e = vdot_c(data.view_z, ring_z);
		double sin_beta_s = vdot_c(data.sun_dir_j2k, ring_z);

		double sin_beta = 0.0;

		if(sin_beta_e * sin_beta_s > 0)
		{
			sin_beta = std::sqrt(std::abs(sin_beta_e * sin_beta_s));
		}
		
		data.magnitude += -8.914 - 1.825 * sin_beta + 0.026 * alpha - 0.378 * sin_beta * std::exp(-2.25 * alpha);
	}
	
	auto GetAA = [&](const char* target, double& alt, double& az, double* v_topo)
	{
		double v_j2k[3], v_fixed[3], v_rel_fixed[3], lt_aa;
		spkpos_c(target, data.et, "J2000", "LT+S", "EARTH", v_j2k, &lt_aa);
		mxv_c(r_j2k_fixed, v_j2k, v_fixed);
		vsub_c(v_fixed, p_obs_fixed, v_rel_fixed);
		v_topo[0] = vdot_c(v_rel_fixed, x_north);
		v_topo[1] = vdot_c(v_rel_fixed, y_west);
		v_topo[2] = vdot_c(v_rel_fixed, z_up);
		double range;
		recazl_c(v_topo, SPICEFALSE, SPICETRUE, &range, &az, &alt);
	};

	double vt_obj[3], vt_sun[3], vt_moon[3];
	GetAA(body.name, data.obj_alt, data.obj_az, vt_obj);
	GetAA("SUN", data.sun_alt, data.sun_az, vt_sun);
	GetAA("MOON", data.moon_alt, data.moon_az, vt_moon);

	if(data.use_refraction)
	{
		double raw_obj_alt = data.obj_alt;
		
		data.obj_alt = ApplyRefraction(data.obj_alt, current_site.alt);
		data.sun_alt = ApplyRefraction(data.sun_alt, current_site.alt);
		data.moon_alt = ApplyRefraction(data.moon_alt, current_site.alt);

		if(std::abs(data.obj_alt - raw_obj_alt) > 1e-12)
		{
			double vt_app[3], v_fixed_app[3], v_j2k_app[3], r_dummy;
			azlrec_c(1.0, data.obj_az, data.obj_alt, SPICEFALSE, SPICETRUE, vt_app);
			
			for(int i=0; i<3; ++i) 
			{
				v_fixed_app[i] = x_north[i]*vt_app[0] + y_west[i]*vt_app[1] + z_up[i]*vt_app[2];
			}
				
			mxv_c(r_fixed_j2k, v_fixed_app, v_j2k_app);
			recrad_c(v_j2k_app, &r_dummy, &data.ra, &data.dec);
		}
	}

	auto ProjectMapInternal = [&](double az, double alt) -> ImVec2
	{
		double r_norm = (90.0 - alt * dpr_c()) / 90.0;
		return ImVec2((float)(-r_norm * sin(az)), (float)(-r_norm * cos(az)));
	};

	ImVec2 m_pos = ProjectMapInternal(data.moon_az, data.moon_alt);

	double m_unit[3], s_unit[3], p_topo[3], p_r, p_az, p_el;
	vhat_c(vt_moon, m_unit);
	vhat_c(vt_sun, s_unit);
	
	vlcom_c(1.0, m_unit, 0.001, s_unit, p_topo);
	recazl_c(p_topo, SPICEFALSE, SPICETRUE, &p_r, &p_az, &p_el);
	if(data.use_refraction) p_el = ApplyRefraction(p_el);

	ImVec2 p_pos = ProjectMapInternal(p_az, p_el);

	data.moon_pa = atan2(p_pos.y - m_pos.y, p_pos.x - m_pos.x);

	double h_deg = data.sun_alt * dpr_c();
	if(h_deg > -0.833)
	{
		data.twilight_state = "DAY";
		data.state_color = ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
	}
	else if(h_deg > -6.0)
	{
		data.twilight_state = "CIVIL TWILIGHT";
		data.state_color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
	}
	else if(h_deg > -12.0)
	{
		data.twilight_state = "NAUTICAL TWILIGHT";
		data.state_color = ImVec4(0.4f, 0.5f, 1.0f, 1.0f);
	}
	else if(h_deg > -18.0) 
	{
		data.twilight_state = "ASTRONOMICAL TWILIGHT";
		data.state_color = ImVec4(0.6f, 0.5f, 0.9f, 1.0f); 
	}
	else
	{
		data.twilight_state = "NIGHT";
		data.state_color = ImVec4(0.8f, 0.8f, 0.9f, 1.0f); 
	}

	data.airmass = (data.obj_alt > 0.0) ? 1.0 / (sin(data.obj_alt) + 0.50572 * pow(data.obj_alt * dpr_c() + 6.07995, -1.6364)) : -1.0;

	double moon_phase_ang = phaseq_c(data.et, "MOON", "SUN", "EARTH", "LT+S");
	data.moon_illumination = (1.0 + cos(moon_phase_ang)) / 2.0;

	double m_pos_ecl[3], s_pos_ecl[3], m_lt, s_lt;
	spkpos_c("MOON", data.et, "ECLIPJ2000", "LT+S", "EARTH", m_pos_ecl, &m_lt);
	spkpos_c("SUN",  data.et, "ECLIPJ2000", "LT+S", "EARTH", s_pos_ecl, &s_lt);
	double mr, mlat, mlon, sr, slat, slon;
	reclat_c(m_pos_ecl, &mr, &mlon, &mlat); reclat_c(s_pos_ecl, &sr, &slon, &slat);
	double p_diff = mlon - slon;
	while (p_diff < 0) p_diff += twopi_c();
	data.moon_age = p_diff * (29.530588853 / twopi_c());

	double s_p_p[3], e_p_r[3];
	spkpos_c("SUN", data.et, "J2000", "LT+S", body.name, s_p_p, &lt);
	spkpos_c("EARTH", data.et, "J2000", "LT+S", body.name, e_p_r, &lt);
	double zs[3]; vhat_c(e_p_r, zs);
	double nv[3] = {0,0,1}, ys[3], xs[3];
	vlcom_c(1.0, nv, -vdot_c(nv, zs), zs, ys); vhat_c(ys, ys); vcrss_c(ys, zs, xs); vhat_c(xs, xs);
	double su[3]; vhat_c(s_p_p, su);
	data.sun_dir_x = vdot_c(su, xs); data.sun_dir_y = vdot_c(su, ys); data.sun_dir_z = vdot_c(su, zs);
	data.illumination = (1.0 + data.sun_dir_z) / 2.0;
	data.angular_size = (2.0 * body.re / data.dist_pe) * 206265.0;

	double m_p2j[3][3], m_j2p[3][3], v_np_p[3] = {0, 0, 1}, v_np_j2k[3];
	pxform_c(body.frame, "J2000", data.et, m_p2j);
	invert_c(m_p2j, m_j2p);
	mxv_c(m_p2j, v_np_p, v_np_j2k);
	data.np_angle = atan2(vdot_c(v_np_j2k, xs), vdot_c(v_np_j2k, ys)) * dpr_c();

	auto ProjectPlanet = [&](double lon, double lat, ImVec2& out) -> bool
	{
		double pf[3];
		double pj[3];
		double radii[3] = {body.re, body.re, body.rp};
		
		latrec_c(1.0, lon, lat, pf);

		for(int j = 0; j < 3; ++j)
		{
			pf[j] *= radii[j];
		}
		
		mxv_c(m_p2j, pf, pj);

		double v_p2e[3];
		vsub_c(pj, pj, v_p2e);

		if(vdot_c(pj, zs) < 0)
		{
			return false;
		}

		out.x = (float)(vdot_c(pj, xs) / body.re);
		out.y = (float)(vdot_c(pj, ys) / body.re);
		return true;
	};
	
	for (int h = 0; h < 5; ++h)
	{
		double lat = (60.0 - h * 30.0) * rpd_c(); 
		
		for (int i = 0; i < 100; ++i)
		{
			double lon = (twopi_c() * i) / 99.0;

			if(!ProjectPlanet(lon, lat, data.lat_pts[h][i]))
			{
				data.lat_pts[h][i] = ImVec2(0, 0);
			}
		}
	}

	for (int l = 0; l < 12; ++l)
	{
		double lon = (l * 30.0) * rpd_c();
		
		for (int i = 0; i < 100; ++i)
		{
			double lat = -halfpi_c() + (pi_c() * i) / 99.0;

			if(!ProjectPlanet(lon, lat, data.lon_pts[l][i]))
			{
				data.lon_pts[l][i] = ImVec2(0, 0);
			}
		}
	}

	double sj[3], sf[3], lons, lats, ds;
	spkpos_c("SUN", data.et, "J2000", "LT+S", body.name, sj, &lt);
	mxv_c(m_j2p, sj, sf);
	reclat_c(sf, &ds, &lons, &lats);

	double ls = (data.target_index == 1) ? -1.0 : 1.0;

	for (int h = 0; h < 12; ++h)
	{
		double lt_h = h * 2.0;
		double l_target = lons + ls * (lt_h - 12.0) * (pi_c() / 12.0);

		for (int i = 0; i < 100; ++i)
		{
			if(!ProjectPlanet(l_target, (pi_c()*(i/99.0-0.5)), data.lt_mer_pts[h][i]))
			{
				data.lt_mer_pts[h][i] = ImVec2(0, 0);
			}
		}
	}

	if(std::strcmp(body.label, "Mars") == 0)
	{
		double state[6], lt;
		spkezr_c("4", data.et, "J2000", "LT+S", "10", state, &lt);
		double r[3] = {state[0], state[1], state[2]};
		double v[3] = {state[3], state[4], state[5]};

		double h[3];
		vcrss_c(r, v, h);
		vhat_c(h, h);

		double tipm[3][3];
		double z_fixed[3] = {0, 0, 1.0};
		double p[3];
		
		pxform_c("IAU_MARS", "J2000", data.et, tipm);
		mxv_c(tipm, z_fixed, p); 
		vhat_c(p, p);

		double e[3];
		vcrss_c(p, h, e);
		vhat_c(e, e);

		double s[3];
		vminus_c(r, s);
		vhat_c(s, s);

		double hxe[3];
		vcrss_c(h, e, hxe);
		
		double x = vdot_c(s, e);
		double y = vdot_c(s, hxe);
		
		data.ls_deg = atan2(y, x) * dpr_c();

		if(data.ls_deg < 0) data.ls_deg += 360.0;
	}

	{
		double lt_to_obs, r, lon, lat;
		double j_pos_to_obs[3], j_pos_to_sun[3];
		double tipm[3][3];

		spkpos_c(body.name, data.et, "J2000", "LT", "399", j_pos_to_obs, &lt_to_obs);
		
		double et_at_target = data.et - lt_to_obs;
		
		pxform_c("J2000", body.frame, et_at_target, tipm);
		
		double j_earth_vec[3] = {-j_pos_to_obs[0], -j_pos_to_obs[1], -j_pos_to_obs[2]};
		double pos_sep[3];
		mxv_c(tipm, j_earth_vec, pos_sep);
		
		reclat_c(pos_sep, &r, &lon, &lat);
		data.sep_lat = lat * dpr_c();
		double east_lon_sep = lon * dpr_c();
		if(east_lon_sep < 0) east_lon_sep += 360.0;
		
		data.sep_lon = east_lon_sep;

		double lt_to_sun;
		spkpos_c("10", et_at_target, "J2000", "LT", body.name, j_pos_to_sun, &lt_to_sun);
		
		double pos_ssp[3];
		mxv_c(tipm, j_pos_to_sun, pos_ssp);
		
		reclat_c(pos_ssp, &r, &lon, &lat);
		data.ssp_lat = lat * dpr_c();
		double east_lon_ssp = lon * dpr_c();
		if(east_lon_ssp < 0) east_lon_ssp += 360.0;
		data.ssp_lon = east_lon_ssp;
	}

	double lt_epr;
	spkpos_c("EARTH", data.et, "J2000", "LT+S", body.name, e_p_r, &lt_epr);
	vhat_c(e_p_r, zs); 
	
	vlcom_c(1.0, nv, -vdot_c(nv, zs), zs, ys); 
	vhat_c(ys, ys);
	vcrss_c(ys, zs, xs); 
	vhat_c(xs, xs);
	
	vequ_c(xs, data.view_x);
	vequ_c(ys, data.view_y);
	vequ_c(zs, data.view_z);

	if(data.target_index == 3)
	{
		double pos_jup[3], lt_jup;
		spkpos_c("5", data.et, "J2000", "LT+S", "EARTH", pos_jup, &lt_jup);

		for (int i = 0; i < 4; ++i)
		{
			double pos_sat[3], lt_sat, pos_rel[3];
			spkpos_c(data.galilean_moons[i].id, data.et, "J2000", "LT+S", "EARTH", pos_sat, &lt_sat);
			
			vsub_c(pos_sat, pos_jup, pos_rel);
			vequ_c(pos_rel, data.galilean_moons[i].pos_rel);

			data.galilean_moons[i].screen_pos[0] = (float)vdot_c(pos_rel, xs);
			data.galilean_moons[i].screen_pos[1] = (float)vdot_c(pos_rel, ys);
			data.galilean_moons[i].z_depth = vdot_c(pos_rel, zs);

			double dist_xy = sqrt(pow(data.galilean_moons[i].screen_pos[0], 2) + pow(data.galilean_moons[i].screen_pos[1], 2));
			data.galilean_moons[i].is_visible = !(dist_xy < body.re && data.galilean_moons[i].z_depth < 0); // should be fixed
		}
	}

	pxform_c(body.frame, "J2000", data.et, data.m_p2j);
	
	data.v_np_j2k[0] = data.m_p2j[0][2];
	data.v_np_j2k[1] = data.m_p2j[1][2];
	data.v_np_j2k[2] = data.m_p2j[2][2];
	
	double s_p_p_pos[3], lt_s;
	spkpos_c("SUN", data.et, "J2000", "LT+S", body.name, s_p_p_pos, &lt_s);
	vhat_c(s_p_p_pos, data.sun_dir_j2k);
}

void SearchPlanetEvent(ObservationData& d, std::string type, int direction)
{
	struct SearchParams { double step; double offset; double window; };
	static const SearchParams p_table[] =
	{
		{ 3.0 * 3600.0,  6.0 * 3600.0, 130.0 * 86400.0}, // 0: Mercury
		{ 6.0 * 3600.0, 12.0 * 3600.0, 600.0 * 86400.0}, // 1: Venus
		{24.0 * 3600.0, 24.0 * 3600.0, 850.0 * 86400.0}, // 2: Mars
		{48.0 * 3600.0, 24.0 * 3600.0, 450.0 * 86400.0}  // 3: Jupiter
	};
	
	const auto& p = p_table[(d.target_index >= 0 && d.target_index < 4) ? d.target_index : 1];
	const BodyConsts& body = bodies[d.target_index];

	const int MAX_INTERVALS = 1000;
	SPICEDOUBLE_CELL(cnfine, MAX_INTERVALS);
	SPICEDOUBLE_CELL(result, MAX_INTERVALS);
	
	double start = d.et + (direction * p.offset);
	double end   = d.et + (direction * p.window);
	
	scard_c(0, &cnfine);
	if(direction > 0) wninsd_c(start, end, &cnfine);
	else wninsd_c(end, start, &cnfine);

	if(type == "INF_CONJ" || type == "SUP_CONJ" || type == "OPPOSITION" || type == "CONJUNCTION")
	{
		if(d.target_index <= 1)
		{// 内惑星
			const char* relate = (type == "INF_CONJ") ? "LOCMAX" : "LOCMIN";
			gfpa_c(body.name, "SUN", "LT+S", "EARTH", relate, 0.0, 0.0, p.step, MAX_INTERVALS, &cnfine, &result);
		}
		else
		{// 外惑星
			const char* relate = (type == "OPPOSITION") ? "LOCMAX" : "LOCMIN";
			gfsep_c(body.name, "POINT", " ", "SUN", "POINT", " ", "LT+S", "EARTH", relate, 0.0, 0.0, p.step, MAX_INTERVALS, &cnfine, &result);
		}
	} 
	else if(type.rfind("MAX_", 0) == 0)
	{// 最大離角
		if(d.target_index <= 1)
		{
			gfsep_c(body.name, "POINT", " ", "SUN", "POINT", " ", "LT+S", "EARTH", "LOCMAX", 0.0, 0.0, p.step, MAX_INTERVALS, &cnfine, &result);
		}
	}

	int count = wncard_c(&result);
	if(count > 0)
	{
		for (int i = 0; i < count; ++i)
		{
			int idx = (direction > 0) ? i : (count - 1 - i);
			double et_found, finish;
			wnfetd_c(&result, idx, &et_found, &finish);

			if(type == "MAX_EAST" || type == "MAX_WEST")
			{
				double s_pos[3], p_pos[3], lt_dummy;
				spkpos_c("SUN",     et_found, "ECLIPJ2000", "LT+S", "EARTH", s_pos, &lt_dummy);
				spkpos_c(body.name, et_found, "ECLIPJ2000", "LT+S", "EARTH", p_pos, &lt_dummy);
				double s_lon, s_lat, s_dist, p_lon, p_lat, p_dist;
				recsph_c(s_pos, &s_dist, &s_lat, &s_lon);
				recsph_c(p_pos, &p_dist, &p_lat, &p_lon);
				double diff = p_lon - s_lon;
				while (diff >  pi_c()) diff -= twopi_c();
				while (diff < -pi_c()) diff += twopi_c();
				if(type == "MAX_EAST" && diff < 0) continue;
				if(type == "MAX_WEST" && diff > 0) continue;
			}

			d.et = et_found;
			char utcstr[64];
			et2utc_c(d.et, "ISOC", 0, 64, utcstr);
			d.tp = chronoflux::TimePoint(std::string(utcstr), "%4Y-%2m-%2dT%2H:%2M:%2S");
			d.is_realtime = false;
			return; 
		}
	}
}

double GetSunAltAt(double et, int site_idx)
{
	const Site& current_site = sites[site_idx];
	double re = 6378.137, rp = 6356.7523;
	double p_obs_fixed[3];
	
	georec_c(current_site.lon * rpd_c(), current_site.lat * rpd_c(), current_site.alt, re, (re - rp) / re, p_obs_fixed);

	double lt, v_j2k[3], r_j2k_fixed[3][3], v_fixed[3], v_rel_fixed[3];
	spkpos_c("SUN", et, "J2000", "LT+S", "EARTH", v_j2k, &lt);
	pxform_c("J2000", "ITRF93", et, r_j2k_fixed);
	mxv_c(r_j2k_fixed, v_j2k, v_fixed);
	vsub_c(v_fixed, p_obs_fixed, v_rel_fixed);

	double z_up[3], y_west[3], x_north[3], z_axis[3] = {0, 0, 1};
	surfnm_c(re, re, rp, p_obs_fixed, z_up); 
	vcrss_c(z_up, z_axis, y_west); vhat_c(y_west, y_west);
	vcrss_c(y_west, z_up, x_north); vhat_c(x_north, x_north);

	double v_topo[3] = {vdot_c(v_rel_fixed, x_north), vdot_c(v_rel_fixed, y_west), vdot_c(v_rel_fixed, z_up)};
	double range, az, alt;
	recazl_c(v_topo, SPICEFALSE, SPICETRUE, &range, &az, &alt);
	return alt;
}

void SearchSolarAltitudeEvent(ObservationData& d, double target_deg, int time_dir, bool look_for_rising)
{
	double search_start = d.et + (time_dir * 60.0); 
	
	double search_limit = 30.0 * 3600.0; 
	double step = 300.0;

	double t1 = search_start;
	double h1 = GetSunAltAt(t1, d.site_index) * dpr_c() - target_deg;

	for (double dt = step; dt < search_limit; dt += step)
	{
		double t2 = search_start + (dt * time_dir);
		double h2 = GetSunAltAt(t2, d.site_index) * dpr_c() - target_deg;

		if(h1 * h2 < 0.0)
		{
			bool is_rising = (time_dir == 1) ? (h2 > h1) : (h1 > h2);

			if(is_rising == look_for_rising)
			{
				double ta = t1, tb = t2;
				for (int i = 0; i < 20; ++i)
				{
					double mid = (ta + tb) * 0.5;
					double hm = GetSunAltAt(mid, d.site_index) * dpr_c() - target_deg;
					if(hm * h1 < 0.0) tb = mid;
					else ta = mid;
				}
				
				d.et = ta; 
				char utcstr[64];
				et2utc_c(d.et, "ISOC", 0, 64, utcstr);
				d.tp = chronoflux::TimePoint(std::string(utcstr), "%4Y-%2m-%2dT%2H:%2M:%2S");
				d.is_realtime = false;
				return;
			}
		}

		t1 = t2; h1 = h2;
	}
}

void DrawPlanetDisk(ImDrawList* dl, ImVec2 center, const ObservationData& data) 
{
	const BodyConsts& body = bodies[data.target_index];
	const float R_base = 220.0f;

	float scale = 1.0f;

	if(data.show_moons && data.target_index == 3)
	{
		scale = (float)(bodies[data.target_index].re / 1800000.0);
	}
	else if(data.target_index == 4)
	{
		scale = 0.7;
	}

	float R = R_base * scale;
	
	const float k = (float)(body.rp / body.re);
	const float lat_rad = (float)(data.sep_lat * rpd_c());
	const float Ry = R * sqrtf(cosf(lat_rad)*cosf(lat_rad)*k*k + sinf(lat_rad)*sinf(lat_rad));

	struct PlanetColor { float r, g, b; };
	static const PlanetColor p_colors[] =
	{
		{180.0f, 180.0f, 180.0f}, // 0: Mercury
		{255.0f, 255.0f, 235.0f}, // 1: Venus
		{255.0f, 120.0f,  80.0f}, // 2: Mars
		{240.0f, 210.0f, 170.0f}, // 3: Jupiter
		{255.0f, 235.0f, 185.0f}  // 4: Saturn
	};
	
	const auto& base = p_colors[(data.target_index >= 0 && data.target_index < 5) ? data.target_index : 1];

	const bool is_saturn = (data.target_index == 4);

	auto DrawRingsJ2000 = [&](bool front)
	{
		if(!is_saturn) return;

		float phi = (float)(-data.np_angle * rpd_c());
		float cos_p = cosf(phi), sin_p = sinf(phi);
		double k = (double)(body.rp / body.re);
		
		double v_pole[3] = {data.m_p2j[0][2], data.m_p2j[1][2], data.m_p2j[2][2]};
		
		double v_z_dot = vdot_c(v_pole, data.view_z);
		double v_v[3];

		for(int i = 0; i < 3; ++i)
		{
			v_v[i] = v_pole[i] - v_z_dot * data.view_z[i];
		}

		double v_mag = vnorm_c(v_v);
		if(v_mag > 1e-10)
		{
			for(int i = 0; i < 3; ++i)
			{
				v_v[i] /= v_mag;
			}
		}
		else
		{
			v_v[0]=0;
			v_v[1]=1;
			v_v[2]=0;
		}

		double v_u[3];
		vcrss_c(v_v, data.view_z, v_u);

		const int res_theta = 512;
		static std::vector<double> c_t, s_t;

		if(c_t.size() != res_theta + 1)
		{
			c_t.resize(res_theta + 1);
			s_t.resize(res_theta + 1);

			for (int i = 0; i <= res_theta; ++i)
			{
				double lon = (twopi_c() * i) / res_theta;
				c_t[i] = cos(lon);
				s_t[i] = sin(lon);
			}
		}

		struct RingSection
		{
			double r_in, r_out;
			float tau;
			float r, g, b;
			int res_r;
		};

		RingSection sections[] =
		{
			{1.1100, 1.2363, 0.00002f, 60, 60, 70, 8},
			{1.2388, 1.5265, 0.00500f, 70, 75, 85, 16},
			{1.5265, 1.9509, 1.80000f, 245, 235, 205, 64},
			{2.0271, 2.2694, 0.50000f, 210, 205, 195, 32},
			{2.3217, 2.3300, 0.04000f, 180, 180, 180, 4}
		};

		auto Project = [&](double r, double cost, double sint)
		{
			double p_j[3];
			for(int j = 0; j < 3; ++j)
			{
				p_j[j] = r * (cost * data.m_p2j[j][0] + sint * data.m_p2j[j][1]);
			}

			float lx = (float)vdot_c(p_j, v_u) * R;
			float ly = (float)vdot_c(p_j, v_v) * R;

			float rx = lx * cos_p - ly * sin_p;
			float ry = lx * sin_p + ly * cos_p;

			return ImVec2(center.x + rx, center.y - ry);
		};

		double mu = fmax(fabs(vdot_c(data.view_z, v_pole)), 0.001);

		for (const auto& ring : sections)
		{
			float alpha_ring = 1.0f - expf(-ring.tau / (float)mu);
			
			if(alpha_ring < 0.001f)
			{
				continue;
			}

			double dr = (ring.r_out - ring.r_in) / ring.res_r;
			ImU32 b_col = IM_COL32((ImU32)ring.r, (ImU32)ring.g, (ImU32)ring.b, (ImU32)(255 * alpha_ring));
			ImU32 s_col = IM_COL32(0, 0, 0, (ImU32)(255 * alpha_ring));

			for (int i = 0; i < res_theta; ++i)
			{
				double ct1 = c_t[i], st1 = s_t[i], ct2 = c_t[i+1], st2 = s_t[i+1];

				// 深度判定
				double p1[3] = {ct1*data.m_p2j[0][0] + st1*data.m_p2j[0][1], ct1*data.m_p2j[1][0] + st1*data.m_p2j[1][1], ct1*data.m_p2j[2][0] + st1*data.m_p2j[2][1]};
				
				if((front && vdot_c(p1, data.view_z) < 0) || (!front && vdot_c(p1, data.view_z) >= 0))
				{
					continue;
				}

				for (int ir = 0; ir < ring.res_r; ++ir)
				{
					double r1 = ring.r_in + dr * ir, r2 = r1 + dr, rm = r1 + dr * 0.5;
					
					double V[3] = {rm * p1[0], rm * p1[1], rm * p1[2]};
					double S[3] = {data.sun_dir_j2k[0], data.sun_dir_j2k[1], data.sun_dir_j2k[2]};
					double Vs[3] = {V[0], V[1], V[2] / k}, Ss[3] = {S[0], S[1], S[2] / k};
					double qa = vdot_c(Ss, Ss), qb = 2.0 * vdot_c(Vs, Ss), qc = vdot_c(Vs, Vs) - 1.0;
					double disc = qb*qb - 4.0*qa*qc;
					
					bool in_shadow = false;

					if(disc > 0)
					{
						double t = (-qb - sqrt(disc)) / (2.0 * qa);
						if(t < 0) t = (-qb + sqrt(disc)) / (2.0 * qa);
						if(t > 0) in_shadow = true;
					}

					ImVec2 pts[4] = {Project(r1, ct1, st1), Project(r2, ct1, st1), Project(r2, ct2, st2), Project(r1, ct2, st2)};
					dl->AddConvexPolyFilled(pts, 4, in_shadow ? s_col : b_col);
				}
			}
		}
	};

	DrawRingsJ2000(false);

	auto DrawPlanetOutlines = [&](int segments = 128)
	{
		float phi = (float)(-data.np_angle * rpd_c());
		float cos_p = cosf(phi);
		float sin_p = sinf(phi);

		for (int i = 0; i < segments; i++)
		{
			float theta0 = (float)(twopi_c() * i / (float)segments);
			float theta1 = (float)(twopi_c() * (i + 1) / (float)segments);

			float x0_l = R * cosf(theta0);
			float y0_l = Ry * sinf(theta0);
			float x1_l = R * cosf(theta1);
			float y1_l = Ry * sinf(theta1);

			float rx0 = x0_l * cos_p - y0_l * sin_p;
			float ry0 = x0_l * sin_p + y0_l * cos_p;
			float rx1 = x1_l * cos_p - y1_l * sin_p;
			float ry1 = x1_l * sin_p + y1_l * cos_p;

			ImVec2 point0 = ImVec2(center.x + rx0, center.y - ry0);
			ImVec2 point1 = ImVec2(center.x + rx1, center.y - ry1);

			dl->AddLine(point0, point1, IM_COL32(180, 180, 180, 255));
		}
	};

	if(!(data.target_index == 3 && data.show_moons))
	{
		float phi = (float)(-data.np_angle * rpd_c());
		float cos_p = cosf(phi);
		float sin_p = sinf(phi);
		
		for (float y = -Ry; y <= Ry; y += 0.5f)
		{
			float dx = R * sqrtf(fmax(0.0f, 1.0f - (y*y)/(Ry*Ry)));

			for (float x = -dx; x <= dx; x += 0.5f)
			{
				float nx = x / R;
				float ny = y / Ry;
				
				float nz_sq = 1.0f - nx*nx - ny*ny;
				
				if(nz_sq < 0)
				{
					continue;
				}

				float nz = sqrtf(nz_sq);

				float grad_x = nx;
				float grad_y = ny / (Ry/R * Ry/R);
				float grad_z = nz;
				
				float len = sqrtf(grad_x*grad_x + grad_y*grad_y + grad_z*grad_z);
				float v_nx = grad_x / len;
				float v_ny = grad_y / len;
				float v_nz = grad_z / len;

				float v_nx_rot = v_nx * cos_p - v_ny * sin_p;
				float v_ny_rot = v_nx * sin_p + v_ny * cos_p;
				float intensity = v_nx_rot * (float)data.sun_dir_x + v_ny_rot * (float)data.sun_dir_y + v_nz * (float)data.sun_dir_z;

				float phi = (float)(-data.np_angle * rpd_c());
				float cp = cosf(phi), sp = sinf(phi);

				double vx[3], vy[3];
				
				for (int i = 0; i < 3; ++i)
				{
					vx[i] =  data.view_x[i] * cp + data.view_y[i] * sp;
					vy[i] = -data.view_x[i] * sp + data.view_y[i] * cp;
				}

				double P[3];
				
				for (int i = 0; i < 3; ++i)
				{
					P[i] = nx * vx[i] + ny * vy[i] + nz * data.view_z[i];
				}

				if(is_saturn && intensity > 0)
				{
					double ln = vdot_c(data.sun_dir_j2k, data.v_np_j2k);
					if(fabs(ln) > 1e-6)
					{
						double t = -vdot_c(P, data.v_np_j2k) / ln;

						if(t > 0)
						{
							double I[3]; 
							vlcom_c(1.0, P, t, data.sun_dir_j2k, I);
							double d2 = vdot_c(I, I);
							
							float tau = 0.0f;
							if(d2 > 1.5265*1.5265 && d2 < 1.9509*1.9509)
							{
								tau = 1.8000f; // B
							}
							else if(d2 > 2.0271*2.0271 && d2 < 2.2694*2.2694)
							{
								tau = 0.5000f; // A
							}
							else if(d2 > 1.2388*1.2388 && d2 < 1.5265*1.5265)
							{
								tau = 0.0050f; // C
							}
							else if(d2 > 2.3217*2.3217 && d2 < 2.3300*2.3300)
							{
								tau = 0.0500f; // F
							}
							else if(d2 > 1.1100*1.1100 && d2 < 1.2363*1.2363)
							{
								tau = 0.0001f; // D
							}

							if(tau > 0.0f)
							{
								float mu_s = (float)fabs(ln);
								float shadow_transmission = expf(-tau / mu_s);
								
								intensity *= fmax(0.05f, shadow_transmission);
							}
						}
					}
				}

				if(intensity > 0)
				{
					float rx = x * cos_p - y * sin_p;
					float ry = x * sin_p + y * cos_p;

					dl->AddRectFilled(ImVec2(center.x + rx, center.y - ry), ImVec2(center.x + rx + 1.2f, center.y - ry + 1.2f), IM_COL32((ImU32)(base.r * intensity), (ImU32)(base.g * intensity), (ImU32)(base.b * intensity), 255));
				}
			}
		}
	}

	DrawRingsJ2000(true);

	auto DrawLine = [&](const ImVec2* pts, ImU32 col, float thickness)
	{
		for(int i = 0; i < 99; ++i)
		{
			if((pts[i].x != 0.0f || pts[i].y != 0.0f) && (pts[i+1].x != 0.0f || pts[i+1].y != 0.0f))
			{
				dl->AddLine(ImVec2(center.x + pts[i].x * R, center.y - pts[i].y * R), ImVec2(center.x + pts[i+1].x * R, center.y - pts[i+1].y * R), col, thickness);
			}
		}
	};

	if(data.show_latitude)
	{
		const char* lat_names[] = {"+60°", "+30°", "EQ", "-30°", "-60°"};
		const float label_offset = 15.0f;

		for (int h = 0; h < 5; ++h)
		{
			ImU32 col = (h == 2) ? IM_COL32(255, 0, 0, 240) : IM_COL32(255, 0, 0, 200);
			float thick = (h == 2) ? 1.6f : 1.2f;
			DrawLine(data.lat_pts[h], col, thick);

			int right_idx = -1, left_idx = -1;
			float max_x = -2.0f, min_x = 2.0f;

			for (int i = 0; i < 100; ++i)
			{
				ImVec2 p = data.lat_pts[h][i];
				if(p.x == 0.0f && p.y == 0.0f) continue;

				if(p.x > max_x)
				{
					max_x = p.x; right_idx = i;
				}

				if(p.x < min_x)
				{
					min_x = p.x; left_idx = i;
				}
			}

			auto DrawLatLabel = [&](int idx)
			{
				if(idx == -1) return;
				
				ImVec2 p_edge = data.lat_pts[h][idx];
				float len = sqrtf(p_edge.x * p_edge.x + p_edge.y * p_edge.y);
				ImVec2 unit_vec = (len > 1e-6f) ? ImVec2(p_edge.x / len, p_edge.y / len) : ImVec2(idx == right_idx ? 1.0f : -1.0f, 0);

				ImVec2 text_pos = ImVec2(center.x + unit_vec.x * (R + label_offset), center.y - unit_vec.y * (Ry + label_offset));

				char label[16];
				snprintf(label, sizeof(label), "%s", lat_names[h]);
				ImVec2 size = ImGui::CalcTextSize(label);
				
				float draw_x = text_pos.x;
				if (idx == left_idx)
				{
					draw_x = text_pos.x - size.x;
				}
				else if (idx == right_idx)
				{
					draw_x = text_pos.x;
				}
				else
				{
					draw_x = text_pos.x - size.x * 0.5f;
				}

				dl->AddText(ImVec2(draw_x, text_pos.y - size.y * 0.5f), IM_COL32(255, 255, 255, 255), label);
			};

			DrawLatLabel(right_idx);
			DrawLatLabel(left_idx);
		}
	}

	if(data.show_local_time)
	{
		for (int h = 0; h < 12; ++h)
		{
			int hour = h * 2;
			ImU32 col;
			float thick;

			if(hour == 12)
			{
				col = IM_COL32(255, 0, 0, 240);
				thick = 1.6f;
			}
			else if(hour == 0)
			{
				col = IM_COL32(120, 120, 255, 240);
				thick = 1.6f;
			}
			else if(hour < 12)
			{
				col = IM_COL32(245, 130, 32, 240);
				thick = 1.2f;
			}
			else
			{
				col = IM_COL32(44, 201, 14, 240);
				thick = 1.2f;
			}

			DrawLine(data.lt_mer_pts[h], col, thick);

			if(data.lt_mer_pts[h][75].x != 0)
			{
				char label[8]; snprintf(label, sizeof(label), "%dh", hour);
				dl->AddText(ImVec2(center.x + data.lt_mer_pts[h][75].x * R + 3, center.y - data.lt_mer_pts[h][75].y * R), col, label);
			}
		}
	}

	if(data.show_longitude)
	{
		for (int l = 0; l < 12; ++l)
		{
			int degrees = l * 30;
			ImU32 col = IM_COL32(255, 0, 0, 240);
			float thick = 1.2f;

			DrawLine(data.lon_pts[l], col, thick);

			if(data.lon_pts[l][70].x != 0.0f || data.lon_pts[l][70].y != 0.0f)
			{
				char label[16];
				snprintf(label, sizeof(label), "%d°", degrees);

				ImVec2 pos = ImVec2(center.x + data.lon_pts[l][70].x * R + 3, center.y - data.lon_pts[l][70].y * R);
				
				dl->AddText(pos, col, label);
			}
		}
	}

	if(data.show_outline)
	{
		DrawPlanetOutlines();

		if(!(data.target_index == 3 && data.show_moons))
		{
			dl->AddLine(ImVec2(center.x, center.y - Ry - 40), ImVec2(center.x, center.y - Ry - 10), IM_COL32_WHITE, 1.5f);
			dl->AddText(ImVec2(center.x - 90, center.y - Ry - 55), IM_COL32_WHITE, "North Celestial Pole");
		}
	}
	
	if(data.target_index == 3 && data.show_moons)
	{
		dl->AddCircle(center, R, IM_COL32(180, 180, 180, 255), 128, 1.0f);
	}

	if(data.target_index == 3 && data.show_moons)
	{
		for (int i = 0; i < 4; ++i)
		{
			const auto& m = data.galilean_moons[i];

			if(!m.is_visible) continue;

			float mx = (float)(vdot_c(m.pos_rel, data.view_x) / bodies[data.target_index].re) * R;
			float my = (float)(vdot_c(m.pos_rel, data.view_y) / bodies[data.target_index].re) * R;
			ImVec2 m_pos = ImVec2(center.x + mx, center.y - my);

			float dir = (my < 0.0f) ? -1.0f : 1.0f;
			float line_dist = 30.0f + (i * 30.0f);

			ImVec2 label_pos = ImVec2(m_pos.x, m_pos.y + (line_dist * dir));

			static const ImU32 moon_base_colors[] =
			{
				IM_COL32(255, 240, 150, 255),
				IM_COL32(220, 220, 220, 255),
				IM_COL32(180, 160, 140, 255),
				IM_COL32(130, 120, 110, 255)
			};

			ImU32 col = moon_base_colors[i];
			
			dl->AddCircleFilled(m_pos, 3.0f, col);
			
			ImVec2 line_start = ImVec2(m_pos.x, m_pos.y + (5.0f * dir));
			ImVec2 line_end   = ImVec2(label_pos.x, label_pos.y - (2.0f * dir));
			dl->AddLine(line_start, line_end, col, 1.0f);

			ImVec2 text_size = ImGui::CalcTextSize(m.name);
			float text_y_offset = (dir < 0.0f) ? -text_size.y : 0.0f;
			
			dl->AddText(ImVec2(label_pos.x - text_size.x * 0.5f, label_pos.y + text_y_offset), col, m.name);
		}
	}

	return;
}

double GetAltitude(double et, const char* target, double lat, double lon, double alt_km)
{
	double re = 6378.137;
	double rp = 6356.7523;
	double f  = (re - rp) / re;

	double p_obs_fixed[3];
	georec_c(lon * rpd_c(), lat * rpd_c(), alt_km, re, f, p_obs_fixed);

	double v_j2k[3], lt;
	spkpos_c(target, et, "J2000", "LT+S", "EARTH", v_j2k, &lt);

	double r_j2k_fixed[3][3];
	pxform_c("J2000", "ITRF93", et, r_j2k_fixed);

	double v_fixed[3];
	mxv_c(r_j2k_fixed, v_j2k, v_fixed);

	double v_rel_fixed[3];
	vsub_c(v_fixed, p_obs_fixed, v_rel_fixed);

	double z_up[3], y_west[3], x_north[3];
	double z_axis[3] = {0, 0, 1.0};
	
	surfnm_c(re, re, rp, p_obs_fixed, z_up); 
	
	vcrss_c(z_up, z_axis, y_west); 
	vhat_c(y_west, y_west);
	
	vcrss_c(y_west, z_up, x_north); 
	vhat_c(x_north, x_north);

	double v_topo[3] = {vdot_c(v_rel_fixed, x_north), vdot_c(v_rel_fixed, y_west), vdot_c(v_rel_fixed, z_up)};

	double range, az_rad, alt_rad;
	recazl_c(v_topo, SPICEFALSE, SPICETRUE, &range, &az_rad, &alt_rad);

	return alt_rad * dpr_c();
}

void DrawAltitudeGraph(ImDrawList* dl, ImVec2 pos, ImVec2 size, ObservationData& d)
{
	float window = 48.0f;
	ImU32 state_u32 = ImGui::ColorConvertFloat4ToU32(d.state_color);
	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), state_u32);
	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(15, 15, 20, 255));
	dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));

	const float mid_x = pos.x + size.x * 0.5f;
	const float x_scale = size.x / window;
	const float alt_min = -20.0f, alt_max = 90.0f;
	const float y_scale = size.y / (alt_max - alt_min);
	const float zero_y = pos.y + size.y - (fabsf(alt_min) * y_scale);
	auto get_y = [&](float alt) { return zero_y - (alt * y_scale); };

	double lat = sites[d.site_index].lat;
	double lon = sites[d.site_index].lon;
	double alt_km = sites[d.site_index].alt;

	double start_et = d.et - (window / 2.0 * 3600.0);
	const double lt_offset = (double)d.tz_offsets[d.edit_tz] * 3600.0;
	const double step_s = 6.0 * 3600.0;

	char start_utc_str[32];
	et2utc_c(start_et + lt_offset, "ISOC", 0, 32, start_utc_str);
	int y, m, day, hh, min, ss;
	sscanf(start_utc_str, "%d-%d-%dT%d:%d:%d", &y, &m, &day, &hh, &min, &ss);
	hh = (hh / 6) * 6;
	char clean_utc[32];
	snprintf(clean_utc, sizeof(clean_utc), "%04d-%02d-%02dT%02d:00:00", y, m, day, hh);
	double first_grid_et;
	utc2et_c(clean_utc, &first_grid_et);
	first_grid_et -= lt_offset;

	const int samples = 601;
	const double dt = (window * 3600.0) / (samples - 1);
	float thresholds[] = { -0.833f, -6.0f, -12.0f, -18.0f };

	auto get_sky_color = [&](float h)
	{
		if(h > -0.833f)
		{
			return ImVec4(1.0f, 1.0f, 0.5f, 0.3f);
		}

		if(h > -6.0f)
		{
			return ImVec4(0.5f, 0.8f, 1.0f, 0.3f);
		}

		if(h > -12.0f)
		{
			return ImVec4(0.4f, 0.5f, 1.0f, 0.3f);
		}

		if(h > -18.0f)
		{
			return ImVec4(0.6f, 0.5f, 0.9f, 0.3f);
		}

		return ImVec4(0.1f, 0.1f, 0.2f, 0.3f);
	};

	double prev_h = GetAltitude(start_et, "SUN", lat, lon, alt_km);

	for (int i = 0; i < samples - 1; ++i)
	{
		double t2 = start_et + (i + 1) * dt;
		double h1 = prev_h;
		double h2 = GetAltitude(t2, "SUN", lat, lon, alt_km);
		prev_h = h2;
		float x1 = pos.x + (float)(i * size.x) / (samples - 1);
		float x2 = pos.x + (float)((i + 1) * size.x) / (samples - 1);

		float crossed_th = NAN;

		for (float th : thresholds)
		{
			if((h1 > th && h2 <= th) || (h1 <= th && h2 > th))
			{
				crossed_th = th; break;
			}
		}
		if(!isnan(crossed_th))
		{
			float t_cross = (crossed_th - h1) / (h2 - h1);
			float x_cross = x1 + t_cross * (x2 - x1);
			dl->AddRectFilled(ImVec2(x1, pos.y), ImVec2(x_cross, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(get_sky_color((h1 + crossed_th) * 0.5f)));
			dl->AddRectFilled(ImVec2(x_cross, pos.y), ImVec2(x2, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(get_sky_color((h2 + crossed_th) * 0.5f)));
		}
		else
		{
			dl->AddRectFilled(ImVec2(x1, pos.y), ImVec2(x2, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(get_sky_color((h1 + h2) * 0.5f)));
		}
	}

	for (double t = first_grid_et; t <= start_et + (window * 3600.0); t += step_s / 6.0)
	{
		if(t < start_et - 0.1) 
		{
			continue;
		}

		float tx = pos.x + (float)((t - start_et) / (window * 3600.0)) * size.x;

		if(tx > pos.x + size.x + 0.1) 
		{
			break;
		}

		char utc_str[32];
		char hhmm[8];
		et2utc_c(t + lt_offset, "ISOC", 0, 32, utc_str);
		memcpy(hhmm, utc_str + 11, 5);
		hhmm[5] = '\0';
		
		bool is_midnight = (strcmp(hhmm, "00:00") == 0);

		ImU32 line_col = is_midnight ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 25);
		dl->AddLine(ImVec2(tx, pos.y), ImVec2(tx, pos.y + size.y), line_col, is_midnight ? 1.5f : 1.0f);
	}

	for (double t = first_grid_et; t <= start_et + (window * 3600.0); t += step_s)
	{
		if(t < start_et - 0.1)
		{
			continue;
		}

		float tx = pos.x + (float)((t - start_et) / (window * 3600.0)) * size.x;

		if(tx > pos.x + size.x + 0.1)
		{
			break;
		}

		char utc_str[32];
		char hhmm[8];

		et2utc_c(t + lt_offset, "ISOC", 0, 32, utc_str);
		memcpy(hhmm, utc_str + 11, 5); hhmm[5] = '\0';

		bool is_midnight = (strcmp(hhmm, "00:00") == 0);

		char display_buf[32];

		if(is_midnight)
		{
			snprintf(display_buf, sizeof(display_buf), "%c%c/%c%c", utc_str[5], utc_str[6], utc_str[8], utc_str[9]);
		}
		else
		{
			snprintf(display_buf, sizeof(display_buf), "%s", hhmm);
		}

		ImVec2 ts = ImGui::CalcTextSize(display_buf);
		dl->AddText(ImVec2(tx - ts.x * 0.5f, pos.y + size.y + 5.0f), IM_COL32(180, 180, 180, 255), display_buf);
		
		ImU32 line_col = IM_COL32(255, 255, 255, 100);
		dl->AddLine(ImVec2(tx, pos.y), ImVec2(tx, pos.y + size.y), line_col, is_midnight ? 1.5f : 1.0f);
	}

	const char* current_tz = d.tz_names[d.edit_tz];
	
	ImVec2 tz_text_size = ImGui::CalcTextSize(current_tz);
	ImVec2 tz_pos = ImVec2(pos.x + size.x + 35.0f, pos.y + size.y + 5.0f);
	
	dl->AddText(tz_pos, IM_COL32(180, 180, 180, 255), current_tz);

	for (int rel_h = -48; rel_h <= 48; rel_h += 6)
	{
		float tx = mid_x + (rel_h * x_scale);

		if(tx < pos.x - 1.0f || tx > pos.x + size.x + 1.0f)
		{
			continue;
		}

		ImU32 line_col = (rel_h == 0) ? IM_COL32(255, 0, 0, 200) : IM_COL32(255, 0, 0, 100);
		dl->AddLine(ImVec2(tx, pos.y), ImVec2(tx, pos.y + size.y), line_col, (rel_h == 0) ? 2.0f : 1.0f);

		char rel_label[16];

		if(rel_h == 0)
		{
			snprintf(rel_label, sizeof(rel_label), "Now");
		}
		else
		{
			snprintf(rel_label, sizeof(rel_label), "%+dh", rel_h);
		}

		ImVec2 ts = ImGui::CalcTextSize(rel_label);
		dl->AddText(ImVec2(tx - ts.x * 0.5f, pos.y - ts.y - 7.0f), IM_COL32(200, 200, 200, 255), rel_label);
	}

	for (int deg : {0, 15, 30, 45, 60, 75, 90})
	{
		float ty = get_y((float)deg);
		ImU32 col = (deg == 0) ? IM_COL32(200, 200, 200, 150) : IM_COL32(255, 255, 255, 30);
		dl->AddLine(ImVec2(pos.x, ty), ImVec2(pos.x + size.x, ty), col, (deg == 0) ? 2.0f : 1.0f);
		char buf[8]; snprintf(buf, sizeof(buf), "%d°", deg);
		ImVec2 text_size = ImGui::CalcTextSize(buf);
		dl->AddText(ImVec2(pos.x - text_size.x - 2.0f, ty - text_size.y * 0.5f), IM_COL32(150, 150, 150, 255), buf);
	}

	ImVec2 prev_t;
	ImVec2 prev_s;
	ImVec2 prev_m;
	dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);

	for (int i = 0; i < samples; ++i)
	{
		double t = start_et + i * dt;
		float px = pos.x + i * (size.x / (samples - 1));
		float val_t = (float)GetAltitude(t, bodies[d.target_index].name, lat, lon, alt_km);
		float val_m = (float)GetAltitude(t, "MOON", lat, lon, alt_km);
		float val_s = (float)GetAltitude(t, "SUN", lat, lon, alt_km);
		ImVec2 cur_t = ImVec2(px, get_y(val_t)), cur_m = ImVec2(px, get_y(val_m)), cur_s = ImVec2(px, get_y(val_s));

		if(i > 0)
		{
			dl->AddLine(prev_s, cur_s, IM_COL32(255, 255, 0, 255), 2.0f);
			dl->AddLine(prev_t, cur_t, IM_COL32(0, 255, 255, 255), 2.0f);
			dl->AddLine(prev_m, cur_m, IM_COL32(160, 160, 160, 255), 2.0f);
		}

		prev_t = cur_t; prev_s = cur_s; prev_m = cur_m;
	}

	dl->PopClipRect();
}

void DrawMoonOrbitGraphCustom(ImDrawList* dl, ImVec2 pos, ImVec2 size, ObservationData& d)
{
	float window = 24.0 * 16.0;
	
	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 20, 25, 255));
	dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));

	const float mid_y = pos.y + size.y * 0.5f;
	const float mid_x = pos.x + size.x * 0.5f;
	const float x_scale = size.x / window; 
	
	const float y_limit = 650.0f;
	const float y_scale = (size.y * 0.45f) / y_limit; 
	
	double start_et = d.et - (window / 2.0 * 3600.0);
	const double lt_offset = (double)d.tz_offsets[d.edit_tz] * 3600.0;
	const double step_s = 6.0 * 3600.0;

	char start_utc_str[32];
	et2utc_c(start_et + lt_offset, "ISOC", 0, 32, start_utc_str);

	int y, m, day, hh, min, ss;
	sscanf(start_utc_str, "%d-%d-%dT%d:%d:%d", &y, &m, &day, &hh, &min, &ss);
	hh = (hh / 6) * 6;

	char clean_utc[32];
	snprintf(clean_utc, sizeof(clean_utc), "%04d-%02d-%02dT%02d:00:00", y, m, day, hh);
	double first_grid_et;
	utc2et_c(clean_utc, &first_grid_et);
	first_grid_et -= lt_offset;

	for (double t = first_grid_et; t <= start_et + (window * 3600.0); t += step_s)
	{
		if(t < start_et - 0.1)
		{
			continue;
		}

		float tx = pos.x + (float)((t - start_et) / (window * 3600.0)) * size.x;
		if(tx > pos.x + size.x + 0.1)
		{
			break;
		}

		char utc_str[32];
		char hhmm[8];
		et2utc_c(t + lt_offset, "ISOC", 0, 32, utc_str);
		memcpy(hhmm, utc_str + 11, 5);
		hhmm[5] = '\0';
		
		bool is_midnight = (strcmp(hhmm, "00:00") == 0);

		ImU32 line_col = is_midnight ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 25);
		dl->AddLine(ImVec2(tx, pos.y), ImVec2(tx, pos.y + size.y), line_col, is_midnight ? 1.5f : 1.0f);

		int h_rel = (int)round((t - d.et) / 3600.0);

		if(is_midnight && ((int)floor((t + lt_offset) / 86400.0) % 2 == 0))
		{
			ImVec2 ts = ImGui::CalcTextSize(hhmm);
			char mmdd[10];
			snprintf(mmdd, sizeof(mmdd), "%c%c/%c%c", utc_str[5], utc_str[6], utc_str[8], utc_str[9]);
			dl->AddText(ImVec2(tx - ts.x * 0.5f, pos.y + size.y + 5.0f), IM_COL32(180, 180, 180, 255), mmdd);
		}
	}

	const char* current_tz = d.tz_names[d.edit_tz];
	
	ImVec2 tz_text_size = ImGui::CalcTextSize(current_tz);
	ImVec2 tz_pos = ImVec2(pos.x + size.x + 35.0f, pos.y + size.y + 5.0f);
	
	dl->AddText(tz_pos, IM_COL32(180, 180, 180, 255), current_tz);

	for (int rel_h = -192; rel_h <= 192; rel_h += 48)
	{
		float tx = mid_x + (rel_h * x_scale);
		
		if(tx < pos.x - 1.0f || tx > pos.x + size.x + 1.0f)
		{
			continue;
		}

		ImU32 line_col = (rel_h == 0) ? IM_COL32(255, 0, 0, 200) : IM_COL32(255, 0, 0, 100);
		float thick = (rel_h == 0) ? 2.0f : 1.0f;
		dl->AddLine(ImVec2(tx, pos.y), ImVec2(tx, pos.y + size.y), line_col, thick);

		char rel_label[16];

		if(rel_h == 0)
		{
			snprintf(rel_label, sizeof(rel_label), "Now");
		}
		else
		{
			snprintf(rel_label, sizeof(rel_label), "%+dh", rel_h);
		}
		
		ImVec2 ts = ImGui::CalcTextSize(rel_label);
		dl->AddText(ImVec2(tx - ts.x * 0.5f, pos.y - ts.y - 5.0f), IM_COL32(200, 200, 200, 255), rel_label);
	}

	int y_ticks[] = {0, 200, 400, 600};

	for (int tick : y_ticks)
	{
		for (int sign : {1, -1})
		{
			if(tick == 0 && sign == -1)
			{
				continue;
			}

			float val = (float)tick * sign;
			float ty = mid_y - val * y_scale;

			if(tick != 0) 
			{
				dl->AddLine(ImVec2(pos.x, ty), ImVec2(pos.x + size.x, ty), IM_COL32(255, 255, 255, 30));
			}

			char buf[16];
			snprintf(buf, sizeof(buf), "%.0f\"", val);
			ImVec2 l_size = ImGui::CalcTextSize(buf);
			dl->AddText(ImVec2(pos.x - l_size.x - 8.0f, ty - l_size.y * 0.5f), IM_COL32(150, 150, 150, 255), buf);
			
			if(tick == 0)
			{
				break;
			}
		}
	}

	static const ImU32 colors[] =
	{
		IM_COL32(255, 240, 150, 255), IM_COL32(220, 220, 220, 255),
		IM_COL32(180, 160, 140, 255), IM_COL32(130, 120, 110, 255)
	};

	const int samples = 200;
	const double dt = (window * 3600.0) / (samples - 1);

	ImVec2 shadow_top[samples];
	ImVec2 shadow_bottom[samples];

	for (int i = 0; i < samples; ++i)
	{
		double t = start_et + i * dt;
		double pos_j[3], lt_j;
		spkpos_c("5", t, "J2000", "LT+S", "EARTH", pos_j, &lt_j);
		
		double dist_earth = vnorm_c(pos_j);
		float ang_r = (float)((69911.0 / dist_earth) * 206265.0);
		
		float px = pos.x + i * (size.x / (samples - 1));
		shadow_top[i] = ImVec2(px, mid_y - ang_r * y_scale);
		shadow_bottom[i] = ImVec2(px, mid_y + ang_r * y_scale);
	}

	for (int i = 0; i < samples - 1; ++i)
	{
		dl->AddRectFilledMultiColor(shadow_top[i], shadow_bottom[i + 1], IM_COL32(60, 60, 70, 180), IM_COL32(60, 60, 70, 180), IM_COL32(60, 60, 70, 180), IM_COL32(60, 60, 70, 180));
	}

	for (int j = 0; j < 4; ++j)
	{
		ImVec2 prev_p;

		for (int i = 0; i < samples; ++i)
		{
			double t = start_et + i * dt;
			double pos_j[3], pos_s[3], pos_rel[3], lt;
			spkpos_c("5", t, "J2000", "LT+S", "EARTH", pos_j, &lt);
			spkpos_c(d.galilean_moons[j].id, t, "J2000", "LT+S", "EARTH", pos_s, &lt);
			vsub_c(pos_s, pos_j, pos_rel);
			
			double dist_earth = vnorm_c(pos_j);
			double offset_arcsec = (vdot_c(pos_rel, d.view_x) / dist_earth) * 206265.0;
			
			ImVec2 cur_p = ImVec2(pos.x + i * (size.x / (samples-1)), mid_y - (float)offset_arcsec * y_scale);
			if(i > 0) dl->AddLine(prev_p, cur_p, colors[j], 1.5f);
			prev_p = cur_p;
		}
		
		dl->AddText(ImVec2(pos.x + size.x + 8.0f, prev_p.y - 7.0f), colors[j], d.galilean_moons[j].name);
	}
}

void DrawCelestialMap(ImDrawList* dl, ImVec2 center, const ObservationData& data)
{
	const float R = 160.0f; 
	
	ImVec4 bg_v4 = data.state_color;
	bg_v4.x *= 0.15f; bg_v4.y *= 0.15f; bg_v4.z *= 0.15f;
	ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(bg_v4);

	dl->AddCircleFilled(center, R, bg_color);
	dl->AddCircle(center, R, IM_COL32(100, 100, 100, 255), 64, 2.0f); 

	ImVec2 text_pos = ImVec2(center.x - R - 35.0f, center.y - R - 42.0f);

	dl->AddText(text_pos, IM_COL32(200, 200, 200, 255), "Lighting: ");
	
	float label_width = ImGui::CalcTextSize("Lighting: ").x;
	ImVec2 state_pos = ImVec2(text_pos.x + label_width, text_pos.y);
	
	ImU32 state_u32 = ImGui::ColorConvertFloat4ToU32(data.state_color);
	dl->AddText(state_pos, state_u32, data.twilight_state.c_str());

	dl->AddLine(ImVec2(center.x, center.y - R), ImVec2(center.x, center.y + R), IM_COL32(50, 50, 60, 255), 1.0f);
	dl->AddLine(ImVec2(center.x - R, center.y), ImVec2(center.x + R, center.y), IM_COL32(50, 50, 60, 255), 1.0f);
	
	struct Label
	{
		const char* txt;
		ImVec2 pos;
	};

	static const Label labels[] = {{"N", {0, -1}}, {"W", {1, 0}}, {"S", {0, 1}}, {"E", {-1, 0}}};

	for(const auto& l : labels)
	{
		dl->AddText(ImVec2(center.x + l.pos.x*(R+15) - 5, center.y + l.pos.y*(R+15) - 7), IM_COL32_WHITE, l.txt);
	}

	for(float alt : {30.0f, 60.0f})
	{
		dl->AddCircle(center, R * (90.0f - alt) / 90.0f, IM_COL32(50, 50, 60, 255), 64);
	}

	auto PlotObject = [&](double az_rad, double alt_rad, ImU32 col, const char* name, bool is_sun = false, bool is_moon = false)
	{
		double alt_deg = alt_rad * dpr_c();
		if(alt_deg < -1.5) return; 

		float r = R * (float)(90.0 - alt_deg) / 90.0f;
		float x = center.x - r * sin((float)az_rad);
		float y = center.y - r * cos((float)az_rad);
		ImVec2 pos = ImVec2(x, y);

		if(is_sun)
		{
			dl->AddCircleFilled(pos, 8.0f, IM_COL32(255, 255, 0, 255));
		}
		else if(is_moon)
		{
			const float mR = 8.0f;
			dl->AddCircleFilled(pos, mR, IM_COL32(40, 40, 50, 255));
			float t_k = (float)(1.0 - 2.0 * data.moon_illumination);
			float angle = (float)data.moon_pa;
			float cos_a = cos(angle), sin_a = sin(angle);

			auto Rotate = [&](float lx, float ly)
			{ 
				return ImVec2(pos.x + (lx * cos_a - ly * sin_a), pos.y + (lx * sin_a + ly * cos_a)); 
			};

			for (float dy = -mR; dy <= mR; dy += 0.5f)
			{
				float dx_edge = sqrt(fmax(0.0f, mR * mR - dy * dy));
				dl->AddLine(Rotate(dx_edge * t_k, dy), Rotate(dx_edge, dy), col);
			}

			dl->AddCircle(pos, mR, IM_COL32(200, 200, 200, 100), 32);
			char age_label[16]; snprintf(age_label, sizeof(age_label), "Moon (%.1f)", data.moon_age);
			dl->AddText(ImVec2(pos.x + 12, pos.y - 10), col, age_label);
			return;
		}
		else
		{
			dl->AddCircleFilled(pos, 5.0f, col);
		}

		dl->AddText(ImVec2(x + 10, y - 10), col, name);
	};

	PlotObject(data.sun_az, data.sun_alt, IM_COL32(255, 255, 140, 255), "SUN", true);
	
	static const ImU32 p_colors[] =
	{
		IM_COL32(200, 200, 200, 255), // Mercury
		IM_COL32(255, 220, 100, 255), // Venus
		IM_COL32(255, 100,  80, 255), // Mars
		IM_COL32(240, 200, 160, 255), // 3: Jupiter
		IM_COL32(255, 235, 185, 255)  // 4: Saturn
	};

	ImU32 obj_col = p_colors[(data.target_index >= 0 && data.target_index < 4) ? data.target_index : 1];

	PlotObject(data.obj_az, data.obj_alt, obj_col, bodies[data.target_index].label);
	PlotObject(data.moon_az, data.moon_alt, IM_COL32(220, 220, 235, 255), "MOON", false, true);
}

template<typename AddFunc> bool TimeFieldWithButtons(const char* label, int* value, AddFunc add_func, float width = 50.0f)
{
	bool changed = false;
	ImGui::PushID(label);
	
	ImGui::PushButtonRepeat(true);
	
	int base_val = *value;

	if(ImGui::Button("-"))
	{
		add_func(-1); changed = true;
	}
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(width);

	if(ImGui::InputInt("##val", value, 0, 0))
	{
		// 入力中
	}

	if(ImGui::IsItemDeactivatedAfterEdit())
	{
		int delta = *value - base_val;
		if(delta != 0)
		{
			add_func(delta);
			changed = true;
		}
	}
	
	ImGui::SameLine();

	if(ImGui::Button("+"))
	{
		add_func(1); changed = true;
	}
	
	ImGui::PopButtonRepeat();
	
	ImGui::SameLine();
	ImGui::Text("%s", label);
	
	ImGui::PopID();
	return changed;
}

void RenderGUI(ObservationData& d)
{
	const BodyConsts& body = bodies[d.target_index];
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	float margin = 10.0f;
	float full_height = main_viewport->WorkSize.y - (margin * 2.0f);

	auto formatRA = [](double ra_rad)
	{
		double ra_h = ra_rad * 12.0 / pi_c();
		if(ra_h < 0) ra_h += 24.0;
		int h = (int)ra_h;
		int m = (int)((ra_h - h) * 60);
		double s = (ra_h - h - m/60.0) * 3600;
		char buf[32]; 
		snprintf(buf, sizeof(buf), "%02dh %02dm %05.2fs", h, m, s);
		return std::string(buf);
	};

	auto formatDec = [](double dec_rad)
	{
		double dec_d = dec_rad * dpr_c();
		char sign = (dec_d >= 0) ? '+' : '-';
		dec_d = std::abs(dec_d);
		int d = (int)dec_d;
		int m = (int)((dec_d - d) * 60);
		double s = (dec_d - d - m/60.0) * 3600;
		char buf[32]; 
		snprintf(buf, sizeof(buf), "%c%02d° %02d' %05.2f\"", sign, d, m, s);
		return std::string(buf);
	};

	ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, full_height), ImGuiCond_Always);

	if(ImGui::Begin("Configuration", nullptr, window_flags))
	{
		ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "TARGET SELECTION");
		static const char* target_list[] = {"Mercury", "Venus", "Mars", "Jupiter", "Saturn"};
		if(ImGui::Combo("##Target Planet", &d.target_index, target_list, 5))
		{
			CalculateObservation(d);
		}
		ImGui::Separator();

		ImGui::TextColored(ImVec4(0, 1, 1, 1), "TIME MANAGEMENT");
		if(ImGui::Checkbox("Real-time Update", &d.is_realtime))
		{
			if(d.is_realtime) d.tp = chronoflux::now(0.0);
		}

		ImGui::Text("UT : %s", d.tp.format("%4Y/%2m/%2d %2H:%2M:%2S").c_str());
		auto jst = d.tp; jst.addHours(9.0);
		ImGui::Text("JST: %s", jst.format("%4Y/%2m/%2d %2H:%2M:%2S").c_str());
		auto hst = d.tp; hst.addHours(-10.0);
		ImGui::Text("HST: %s", hst.format("%4Y/%2m/%2d %2H:%2M:%2S").c_str());

		ImGui::Separator();

		auto edit_tp = d.tp;
		edit_tp.addHours(d.tz_offsets[d.edit_tz]);

		int y, m, day, h, min; double s;
		edit_tp.extractCalendarFields(y, m, day, h, min, s);
		int s_int = static_cast<int>(s);

		auto update_tp = [&]()
		{
			d.tp = edit_tp;
			d.tp.addHours(-d.tz_offsets[d.edit_tz]);
			d.is_realtime = false;
		};

		for (int i = 0; i < 3; ++i)
		{
			if(ImGui::RadioButton(d.tz_names[i], &d.edit_tz, i))
			{
				;
			}
			if(i < 2) ImGui::SameLine();
		}
		
		if(TimeFieldWithButtons("Y", &y, [&](int v)
		{
			y += v;
			if(y < 1970) y = 1970;
			if(y > 2120) y = 2120;
			edit_tp = chronoflux::TimePoint(y, m, day, h, min, s);
			update_tp();
		}))
		{
			if(y < 1970) y = 1970;
			if(y > 2120) y = 2120;
			edit_tp = chronoflux::TimePoint(y, m, day, h, min, s);
			update_tp();
		}

		if(TimeFieldWithButtons("M", &m, [&](int v)
		{
			int ny = y, nm = m + v;
			if(nm < 1) { ny--; nm = 12; } else if(nm > 12) { ny++; nm = 1; }
			int max_d = d.tp.daysInMonth(ny, nm);
			edit_tp = chronoflux::TimePoint(ny, nm, (day > max_d ? max_d : day), h, min, s);
			update_tp();
		}))
		{
			update_tp();
		}

		if(TimeFieldWithButtons("D", &day, [&](int v) { d.tp.addDays(v); d.is_realtime = false; })) d.is_realtime = false;
		if(TimeFieldWithButtons("h", &h, [&](int v) { d.tp.addHours(v); d.is_realtime = false; })) d.is_realtime = false;
		if(TimeFieldWithButtons("m", &min, [&](int v) { d.tp.addMinutes(v); d.is_realtime = false; })) d.is_realtime = false;
		if(TimeFieldWithButtons("s", &s_int, [&](int v) { d.tp.addSeconds(v); d.is_realtime = false; })) d.is_realtime = false;

		ImGui::Separator();
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "OBSERVATION SITE");
		static const char* site_items[8];
		for(int i = 0; i < 8; ++i) site_items[i] = sites[i].name;
		ImGui::SetNextItemWidth(-FLT_MIN);
		if(ImGui::Combo("##Location", &d.site_index, site_items, 8)) CalculateObservation(d);

		const auto& st = sites[d.site_index];
		ImGui::Text("Lon: %7.3f°E | Lat: %+7.3f°", st.lon, st.lat);
		ImGui::Text("Alt: %7.3f km", st.alt);

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "PHYSICAL OPTIONS");
		ImGui::Checkbox("Apply Atmospheric Refraction", &d.use_refraction);

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "KEY EVENTS JUMP");

		if(d.target_index <= 1)
		{ // 内惑星
			auto EventButton = [&](const char* label, std::string type, int dir)
			{
				if(ImGui::Button(label))
				{
					SearchPlanetEvent(d, type, dir);
				}
			};
			
			ImGui::Text("Inferior Conjunction"); ImGui::SameLine();
			EventButton("Prev##Inf", "INF_CONJ", -1); ImGui::SameLine();
			EventButton("Next##Inf", "INF_CONJ",  1);
			
			ImGui::Text("Superior Conjunction"); ImGui::SameLine();
			EventButton("Prev##Sup", "SUP_CONJ", -1); ImGui::SameLine();
			EventButton("Next##Sup", "SUP_CONJ",  1);

			ImGui::Text("Greatest Eastern Elongation"); ImGui::SameLine();
			EventButton("Prev##GEE", "MAX_EAST", -1); ImGui::SameLine();
			EventButton("Next##GEE", "MAX_EAST",  1);
			
			ImGui::Text("Greatest Western Elongation"); ImGui::SameLine();
			EventButton("Prev##GWE", "MAX_WEST", -1); ImGui::SameLine();
			EventButton("Next##GWE", "MAX_WEST",  1);
			
		}
		else
		{ // 外惑星
			auto EB = [&](const char* label, std::string type, int dir)
			{
				if(ImGui::Button(label)) SearchPlanetEvent(d, type, dir);
			};
			ImGui::Text("Opposition  "); ImGui::SameLine();
			EB("Prev##Opp", "OPPOSITION", -1); ImGui::SameLine(); EB("Next##Opp", "OPPOSITION", 1);
			ImGui::Text("Conjunction "); ImGui::SameLine();
			EB("Prev##Conj", "CONJUNCTION", -1); ImGui::SameLine(); EB("Next##Conj", "CONJUNCTION", 1);
		}

		ImGui::Separator();
		ImGui::Text("Lighting: "); ImGui::SameLine();
		ImGui::TextColored(d.state_color, "%s", d.twilight_state.c_str());

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "SOLAR EVENTS JUMP");

		ImGui::Text("Sunrise");ImGui::SameLine();
		if(ImGui::Button("Prev##sunrise"))
		{
			SearchSolarAltitudeEvent(d, -0.833, -1, true);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##sunrise"))
		{
			SearchSolarAltitudeEvent(d, -0.833, 1, true);
		}

		ImGui::Text("Sunset");ImGui::SameLine();
		if(ImGui::Button("Prev##sunset"))
		{
			SearchSolarAltitudeEvent(d, -0.833, -1, false);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##sunset"))
		{
			SearchSolarAltitudeEvent(d, -0.833, 1, false);
		}


		ImGui::Text("Civil Dawn");ImGui::SameLine();
		if(ImGui::Button("Prev##civil_dawn"))
		{
			SearchSolarAltitudeEvent(d, -6.0, -1, true);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##civil_dawn"))
		{
			SearchSolarAltitudeEvent(d, -6.0, 1, true);
		}

		ImGui::Text("Civil Dusk");ImGui::SameLine();
		if(ImGui::Button("Prev##civil_dusk"))
		{
			SearchSolarAltitudeEvent(d, -6.0, -1, false);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##civil_dusk"))
		{
			SearchSolarAltitudeEvent(d, -6.0, 1, false);
		}


		ImGui::Text("Nautical Dawn");ImGui::SameLine();
		if(ImGui::Button("Prev##nautical_dawn"))
		{
			SearchSolarAltitudeEvent(d, -12.0, -1, true);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##nautical_dawn"))
		{
			SearchSolarAltitudeEvent(d, -12.0, 1, true);
		}

		ImGui::Text("Nautical Dusk");ImGui::SameLine();
		if(ImGui::Button("Prev##nautical_dusk"))
		{
			SearchSolarAltitudeEvent(d, -12.0, -1, false);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##nautical_dusk"))
		{
			SearchSolarAltitudeEvent(d, -12.0, 1, false);
		}


		ImGui::Text("Astronomical Dawn");ImGui::SameLine();
		if(ImGui::Button("Prev##astronomical_dawn"))
		{
			SearchSolarAltitudeEvent(d, -18.0, -1, true);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##astronomical_dawn"))
		{
			SearchSolarAltitudeEvent(d, -18.0, 1, true);
		}

		ImGui::Text("Astronomical Dusk");ImGui::SameLine();
		if(ImGui::Button("Prev##astronomical_dusk"))
		{
			SearchSolarAltitudeEvent(d, -18.0, -1, false);
		}

		ImGui::SameLine();
		if(ImGui::Button("Next##astronomical_dusk"))
		{
			SearchSolarAltitudeEvent(d, -18.0, 1, false);
		}

	}

	ImGui::End();

	char disk_title[64]; snprintf(disk_title, 64, "%s Disk View", body.label);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));
	ImGui::SetNextWindowPos(ImVec2(420, 10), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(750, 1010), ImGuiCond_Always);

	if(ImGui::Begin(disk_title, nullptr, window_flags))
	{
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::TextColored(ImVec4(0, 1, 1, 1), "GALILEAN MOONS ELONGATION");
			
			for (const auto& m : d.galilean_moons)
			{
				double dx = vdot_c(m.pos_rel, d.view_x);
				double dy = vdot_c(m.pos_rel, d.view_y);
				
				double dist_rj = sqrt(dx * dx + dy * dy) / bodies[3].re;
				double dist_arcsec = dist_rj * (d.angular_size / 2.0);

				ImGui::Text("%-8s: %5.2f Rj (%5.1f\")", m.name, dist_rj, dist_arcsec);
				ImGui::SameLine(270);
				
				char bar_label[16];
				snprintf(bar_label, sizeof(bar_label), " ");
				ImGui::ProgressBar((float)(dist_rj / 30.0), ImVec2(-1, 14), bar_label);
			}
		}

		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 region = ImGui::GetContentRegionAvail();
		
		if(!(d.target_index == 3 && d.show_moons))
		{
			ImVec2 center = ImVec2(ImGui::GetCursorScreenPos().x + region.x * 0.5f, ImGui::GetCursorScreenPos().y + region.y / 1.6 * 0.45f);
			DrawPlanetDisk(dl, center, d);

			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 460);
			ImGui::Separator();
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImGui::Text("Altitude Graph");
			ImVec2 alt_pos = ImGui::GetCursorScreenPos();
			ImVec2 alt_size = ImVec2(ImGui::GetContentRegionAvail().x - 110, 240);
			DrawAltitudeGraph(dl, ImVec2(alt_pos.x + 40, alt_pos.y + 22), alt_size, d);
		}
		else
		{
			ImVec2 center = ImVec2(ImGui::GetCursorScreenPos().x + region.x * 0.5f, ImGui::GetCursorScreenPos().y + region.y / 2 * 0.45f);
			DrawPlanetDisk(dl, center, d);
		}

		if(d.target_index == 3 && d.show_moons)
		{			
			float graph_height = 280.0f;
			ImVec2 center = ImVec2(ImGui::GetCursorScreenPos().x + 50, ImGui::GetWindowHeight() - 445);
			ImVec2 graph_size = ImVec2(ImGui::GetContentRegionAvail().x - 130, graph_height);
			
			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 500);
			ImGui::Separator();
			ImGui::Text("Galilean Moons Orbit Graph");
			DrawMoonOrbitGraphCustom(dl, center, graph_size, d);
		}

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 145); 

		ImGui::Separator();
		if(std::strcmp(body.label, "Mars") == 0)
		{
			ImGui::Columns(2, nullptr, false);
			ImGui::Text("Sub-Solar Lon: %5.2f°E | Lat: %+5.2f°", d.ssp_lon, d.ssp_lat);
			ImGui::NextColumn();
			ImGui::Text("Martian Solar Longitude Ls: %.2f°", d.ls_deg);
			ImGui::Columns(1);
			ImGui::Text("Sub-Earth Lon: %5.2f°E | Lat: %+5.2f°", d.sep_lon, d.sep_lat);
		}
		else
		{
			ImGui::Text("Sub-Solar Lon: %5.2f°E | Lat: %+5.2f°", d.ssp_lon, d.ssp_lat);
			ImGui::Text("Sub-Earth Lon: %5.2f°E | Lat: %+5.2f°", d.sep_lon, d.sep_lat);
		}
		
		
		ImGui::Separator();
		ImGui::Columns(2, nullptr, false);
		ImGui::Text("Illumination: %.2f%%", d.illumination * 100.0);
		ImGui::NextColumn();
		ImGui::Text("Angular Diameter: %.2f\"", d.angular_size);
		ImGui::Columns(1);
		ImGui::Columns(2, nullptr, false);
		ImGui::Text("North-Pole Angle: %+.2f°", -d.np_angle);
		ImGui::NextColumn();
		ImGui::Text("Visual Magnitude: %+.2f°", d.magnitude);
		ImGui::Columns(1);

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "OVERLAY OPTIONS");
		char eq_label[64], lon_label[64], lt_label[64], ol_label[64];
		snprintf(eq_label, 64, "Latitude");
		snprintf(lon_label, 64, "Longitude");
		snprintf(lt_label, 64, "Local Time");
		snprintf(ol_label, 64, "Outline");
		ImGui::Columns(5, nullptr, false);
		ImGui::SetColumnWidth(3, 120.0f);
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Checkbox(eq_label, &d.show_latitude);
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::EndDisabled();
		}
		ImGui::NextColumn();
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Checkbox(lon_label, &d.show_longitude);
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::EndDisabled();
		}
		ImGui::NextColumn();
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Checkbox(lt_label, &d.show_local_time);
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::EndDisabled();
		}
		ImGui::NextColumn();
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Checkbox(ol_label, &d.show_outline);
		if(d.target_index == 3 && d.show_moons)
		{
			ImGui::EndDisabled();
		}
		ImGui::NextColumn();
		if(d.target_index == 3)
		{
			if(ImGui::Checkbox("Galilean Moons", &d.show_moons))
			{
				if(d.show_moons)
				{
					d.prev_lat = d.show_latitude;
					d.prev_lon = d.show_longitude;
					d.prev_lt = d.show_local_time;
					d.prev_ol = d.show_outline;

					d.show_latitude = false;
					d.show_longitude = false;
					d.show_local_time = false;
					d.show_outline = true;
				}
				else
				{
					d.show_latitude = d.prev_lat;
					d.show_longitude = d.prev_lon;
					d.show_local_time = d.prev_lt;
					d.show_outline = d.prev_ol;
				}
			}
		}
		ImGui::Columns(1);
	}

	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::SetNextWindowPos(ImVec2(1180, 10), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Always);
	if(ImGui::Begin("Orbital Diagram", nullptr, window_flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImVec2 oc = ImVec2(ImGui::GetCursorScreenPos().x + region.x * 0.5f, ImGui::GetCursorScreenPos().y + region.y * 0.4f);
		
		float orbit_ref = (float)fmax(1.0, body.semi_major);
		float scale = 160.0f / orbit_ref;
		
		auto ToScr = [&](double* p)
		{ 
			return ImVec2(oc.x + (p[0]/1.496e8) * scale, oc.y - (p[1]/1.496e8) * scale); 
		};

		auto DrawSampledOrbit = [&](const char* target_name, double period_days, ImU32 color)
		{
			const int samples = 120;
			ImVec2 prev_pt;
			for (int i = 0; i <= samples; ++i)
			{
				double sample_et = d.et - (period_days * 86400.0 * i / (double)samples);
				double p_pos[3], p_lt;
				
				spkpos_c(target_name, sample_et, "ECLIPJ2000", "NONE", "SUN", p_pos, &p_lt);
				ImVec2 cur_pt = ToScr(p_pos);
				
				if(i > 0)
				{
					dl->AddLine(prev_pt, cur_pt, color, 1.0f);
				}

				prev_pt = cur_pt;
			}
		};

		static const ImU32 p_colors[] =
		{
			IM_COL32(200, 200, 200, 255), // Mercury
			IM_COL32(255, 220, 100, 255), // Venus
			IM_COL32(255, 100,  80, 255), // Mars
			IM_COL32(240, 200, 160, 255)  // Jupiter
		};

		ImU32 obj_col = p_colors[(d.target_index >= 0 && d.target_index < 4) ? d.target_index : 1];

		DrawSampledOrbit("EARTH", 365.256, IM_COL32(80, 80, 100, 255));
		DrawSampledOrbit(body.name, body.period, IM_COL32(120, 120, 120, 255));

		dl->AddCircleFilled(oc, 9, IM_COL32(255, 255, 140, 255)); // Sun
		dl->AddCircleFilled(ToScr(d.pos_p_sun), 7, obj_col); // Target
		dl->AddCircleFilled(ToScr(d.pos_e_sun), 7, IM_COL32(100, 160, 255, 255)); // Earth

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 70);
		ImGui::Separator();
		ImGui::Text("Dist (AU): %.7f AU", d.dist_pe / 149597870.7);
		ImGui::Text("Dist (km): %.2f km", d.dist_pe);
		ImGui::Text("Radial Vel: %.5f km/s", d.radial_vel);
	}

	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(1180, 520), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Always);

	if(ImGui::Begin("Celestial Map (Zenith Up)", nullptr, window_flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 start_pos = ImGui::GetCursorScreenPos();
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImVec2 mc = ImVec2(start_pos.x + region.x * 0.5f, start_pos.y + region.y * 0.43f);

		DrawCelestialMap(dl, mc, d);
		
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 70);
		ImGui::Separator();
		ImGui::Columns(2, nullptr, false);
		ImGui::Text("Elongation: %.2f°", d.elongation);
		ImGui::Text("RA:  %s", formatRA(d.ra).c_str());
		ImGui::Text("Dec: %s", formatDec(d.dec).c_str());
		ImGui::NextColumn();
		ImGui::Text("Azimuth:   %.2f°", d.obj_az * dpr_c());
		ImGui::Text("Elevation: %.2f°", d.obj_alt * dpr_c());

		if(d.airmass > 0)
		{
			ImGui::Text("Airmass: %.3f", d.airmass);
		}
		else
		{
			ImGui::Text("Airmass: ---");
		}

		ImGui::Columns(1);
	}

	ImGui::End();
}

double GetCurrentET()
{
	char utc_str[64];
	snprintf(utc_str, sizeof(utc_str), "%s", chronoflux::now().format("%4Y-%2m-%2dT%2H:%2M:%2S").c_str());
	double et;
	str2et_c(utc_str, &et);
	return et;
}

int main(int, char**)
{
	SetupMacOSBundlePath();
	furnsh_c("kernels.tm");

	bodies.resize(5);

	bodies[0].name = "199";
	bodies[1].name = "299";
	bodies[2].name = "4";
	bodies[3].name = "5";
	bodies[4].name = "6";

	bodies[0].label = "Mercury";
	bodies[1].label = "Venus";
	bodies[2].label = "Mars";
	bodies[3].label = "Jupiter";
	bodies[4].label = "Saturn";

	bodies[0].frame = "IAU_MERCURY";
	bodies[1].frame = "IAU_VENUS";
	bodies[2].frame = "IAU_MARS";
	bodies[3].frame = "IAU_JUPITER";
	bodies[4].frame = "IAU_SATURN";

	std::ofstream output("param.txt");

	for (auto& b : bodies)
	{
		SpiceInt id;
		SpiceBoolean found;
		bods2c_c(b.name, &id, &found);

		if(!found)
		{
			output << b.label << ", " << b.name << " not found." << std::endl;
			continue;
		}

		SpiceInt radii_id = (id <= 9) ? (id * 100 + 99) : id;
		SpiceInt n;
		SpiceDouble radii[3];
		bodvcd_c(radii_id, "RADII", 3, &n, radii);
		b.re = radii[0];
		b.rp = radii[2];

		SpiceDouble gm_sun;
		bodvcd_c(10, "GM", 1, &n, &gm_sun);

		SpiceDouble state[6], lt;
		spkezr_c(b.name, 0.0, "ECLIPJ2000", "NONE", "SUN", state, &lt);

		SpiceDouble elts[11];
		oscltx_c(state, 0.0, gm_sun, elts);

		b.period = elts[10] / 86400.0;
		b.semi_major = elts[9] / 149597870.7;
	}

	glfwSetErrorCallback(glfw_error_callback);
	if(!glfwInit()) return 1;

	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1590, 1030, "Planet Observation Planner", NULL, NULL);
	if(window == NULL) return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // VSync

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontGlobalScale = 1.25f;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ObservationData data;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		CalculateObservation(data); 

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		RenderGUI(data);

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		
		glClearColor(0.05f, 0.05f, 0.07f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	unload_c("kernels.tm");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
