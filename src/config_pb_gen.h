#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <google/protobuf/text_format.h>
#include "gen/dsp_test_sweep.pb.h"

class ConfigPbGen {
  public:
    ConfigPbGen(){}

    void createCfg(){
      MainConfig main_cfg;
      createCtrl(main_cfg.mutable_ctrl());
      createTasks(main_cfg.mutable_tasks());
      createOverviewModes(main_cfg.mutable_overview_modes());
      createPulseBursts(main_cfg.mutable_pulse_bursts());
      createSignalIds(main_cfg.mutable_signal_ids());

      std::string textproto_output;
      if (!google::protobuf::TextFormat::PrintToString(main_cfg, &textproto_output)) {
        std::cerr << "Ошибка при конвертации в textproto!" << std::endl;
        return;
      }
      out << textproto_output;
      out.close();
    }

  private:
    void createCtrl(Control* ctrl){
      ctrl->set_dsp_enabled(true);
      ctrl->set_pilot_enabled(false);
      ctrl->set_probe_enabled(false);
      ctrl->set_overview_mode(3);
      ctrl->set_dsp_output_mux(Control_DspOutputMux_DIGITAL_DOWN_CONVERTER);
      ctrl->set_sync_pulse_mux(Control_SyncPulseMux_LOW_LOGIC_LEVEL);
      ctrl->set_tx_output_mux(Control_TxOutputMux_DIGITAL_UP_CONVERTER);
      ctrl->set_tx2rx_latency(0);
      ctrl->set_dec_factor_code(1);
      for(int i = 0; i < 4; ++i)
        ctrl->add_input_mux_rx(static_cast<Control_InputMux>(Control_InputMux_ADC_CHANNEL_0+i));
    }

    void createTasks(Tasks* tasks){
      tasks->set_sm_ctrl(false);
      tasks->set_steering_ctrl(true);

      for(int i = 0; i < 4096; ++i){
        auto que = tasks->add_que();
        que->set_index(i);
        que->set_tag(0);
        que->set_sm(0);
        que->set_active(false);
        que->set_mode_switch(false);
        que->set_az_code(0);
        que->set_el_code(0);
        que->set_fr_code(0);
      }
    }

    void createOverviewModes(OverviewModes* modes){
      for(int i = 1; i < 65; ++i){
        auto* om = modes->add_overview_modes();
        om->set_mid(i);
        om->set_description("");
        for(int j = 1; j < 65; ++j){
          auto* pp = om->add_packet_of_pulses();
          pp->set_steering_ctrl(true);
          pp->set_prb_disable(true);
          pp->set_active(true);
          pp->set_bid(j/2);
          pp->set_index(j);
          pp->set_nor(j);
          pp->set_orthogonal(true);
          pp->set_prp_us(j*0.2);
          pp->set_phase_deg(j*0.1);
          pp->set_qcw(false);
          pp->set_tag(0);
        }
      }
    }


  void createPulseBursts(PulseBursts* pulse_bursts){
    for(int i = 1; i < 65; ++i){
      auto* burst = pulse_bursts->add_burst();
      burst->set_bid(i);
      burst->set_description("");

      for(int j = 1; j < 9; ++j){
        auto* pilot = burst->add_pilot_burst();
        pilot->set_active((j%2 == 0));
        pilot->set_amplitude_db(j*0.2);
        pilot->set_doppler_khz(j*0.2);
        pilot->set_index(j);
        pilot->set_pause_us(j*0.2);
        pilot->set_sid(j);
        pilot->set_sap((j%2 != 0));
        pilot->set_width_us(j*0.2);
      }

      for(int j = 1; j < 9; ++j){
        auto* probe = burst->add_probe_burst();
        probe->set_active((j%2 == 0));
        probe->set_fpf_ns(j*0.2);
        probe->set_rpf_ns(j*0.2);
        probe->set_if_mhz(j*0.2);
        probe->set_index(j);
        probe->set_pause_us(j*0.2);
        probe->set_sid(j);
        probe->set_width_us(j*0.2);
        probe->set_window((j%2 != 0));
      }
    }
  }

  void createSignalIds(SignalIds* signal_ids){
    for(int i = 1; i < 17; ++i){
      auto* si = signal_ids->add_signal_ids();
      si->set_sid(i);

      auto* signal = si->mutable_signal();
      signal->set_scale(i*0.2);
      for(int j = 0; j < 383; ++j){
        signal->add_amplitude_coefs(1.0);
        signal->add_phase_coefs(0.0);
      }
      signal->set_description("");

      auto* xcorr = si->mutable_x_corr();
      xcorr->set_scale(0.5);
      for(int j = 0; j < 383; ++j){
        xcorr->add_amplitude_coefs(1.0);
        xcorr->add_phase_coefs(0.0);
      }

      auto* gauss = si->mutable_gauss_window();
      gauss->set_mean(i*0.2);
      gauss->set_sigma(i*0.2);
      gauss->set_hold(i*0.2);
      gauss->set_stop(i*0.2);

      auto* weight = si->mutable_weight();
      weight->set_scale(0.5);
      for(int j = 0; j < 63; ++j)
        weight->add_filter_coefs(i*0.2);

      auto* ivm = si->mutable_ivm();
      ivm->set_pause_us(i*0.2);
      ivm->set_width_us(i*0.2);
      ivm->set_start_pause_us(i*0.2);
      ivm->set_start_width_us(i*0.2);
    }
  }

  private:
    std::ofstream out{"../../../config_dsp_test_sweep.textproto"};
};
