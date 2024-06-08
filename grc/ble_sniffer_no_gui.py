#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: BLE air sniffer
# Author: Zhongjun Yu
# GNU Radio version: 3.8.5.0

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import zeromq
import iio
import numpy


class ble_sniffer_no_gui(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "BLE air sniffer")

        ##################################################
        # Variables
        ##################################################
        self.transition_width = transition_width = 300e3
        self.sample_rate = sample_rate = 10000000
        self.data_rate = data_rate = 1e6
        self.cutoff_freq = cutoff_freq = 850e3
        self.ble_channel_spacing = ble_channel_spacing = 2e6
        self.ble_channel = ble_channel = 0
        self.ble_base_freq = ble_base_freq = 2402e6
        self.squelch_threshold = squelch_threshold = -70
        self.rf_gain = rf_gain = 100
        self.lowpass_filter = lowpass_filter = firdes.low_pass(1, sample_rate, cutoff_freq, transition_width)
        self.gmsk_sps = gmsk_sps = int(sample_rate/data_rate)
        self.gmsk_omega_limit = gmsk_omega_limit = 0.035
        self.gmsk_mu = gmsk_mu = 0.5
        self.gmsk_gain_mu = gmsk_gain_mu = 0.7
        self.freq_offset = freq_offset = 0
        self.freq = freq = ble_base_freq+(ble_channel_spacing * ble_channel)

        ##################################################
        # Blocks
        ##################################################
        self.zeromq_pub_sink_0 = zeromq.pub_sink(gr.sizeof_char, 1, 'tcp://127.0.0.1:55555', 100, False, -1)
        self.unpacked_to_packed = blocks.unpacked_to_packed_bb(1, gr.GR_LSB_FIRST)
        self.iio_pluto_source_0 = iio.pluto_source('ip:10.88.110.66', int(freq), sample_rate, 2000000, 3276800, True, True, True, 'manual', rf_gain, '', True)
        self.freq_xlating_fir_filter_lp = filter.freq_xlating_fir_filter_ccc(1, lowpass_filter, -freq_offset, sample_rate)
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
            samples_per_symbol=gmsk_sps,
            sensitivity=(numpy.pi*0.5)/gmsk_sps,
            gain_mu=0.175,
            mu=0.5,
            omega_relative_limit=0.005,
            freq_error=0.0,
            verbose=False,
            log=False)
        self.analog_simple_squelch = analog.simple_squelch_cc(squelch_threshold, 0.1)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_simple_squelch, 0), (self.freq_xlating_fir_filter_lp, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.unpacked_to_packed, 0))
        self.connect((self.freq_xlating_fir_filter_lp, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.iio_pluto_source_0, 0), (self.analog_simple_squelch, 0))
        self.connect((self.unpacked_to_packed, 0), (self.zeromq_pub_sink_0, 0))


    def get_transition_width(self):
        return self.transition_width

    def set_transition_width(self, transition_width):
        self.transition_width = transition_width
        self.set_lowpass_filter(firdes.low_pass(1, self.sample_rate, self.cutoff_freq, self.transition_width))

    def get_sample_rate(self):
        return self.sample_rate

    def set_sample_rate(self, sample_rate):
        self.sample_rate = sample_rate
        self.set_gmsk_sps(int(self.sample_rate/self.data_rate))
        self.set_lowpass_filter(firdes.low_pass(1, self.sample_rate, self.cutoff_freq, self.transition_width))
        self.iio_pluto_source_0.set_params(int(self.freq), self.sample_rate, 2000000, True, True, True, 'manual', self.rf_gain, '', True)

    def get_data_rate(self):
        return self.data_rate

    def set_data_rate(self, data_rate):
        self.data_rate = data_rate
        self.set_gmsk_sps(int(self.sample_rate/self.data_rate))

    def get_cutoff_freq(self):
        return self.cutoff_freq

    def set_cutoff_freq(self, cutoff_freq):
        self.cutoff_freq = cutoff_freq
        self.set_lowpass_filter(firdes.low_pass(1, self.sample_rate, self.cutoff_freq, self.transition_width))

    def get_ble_channel_spacing(self):
        return self.ble_channel_spacing

    def set_ble_channel_spacing(self, ble_channel_spacing):
        self.ble_channel_spacing = ble_channel_spacing
        self.set_freq(self.ble_base_freq+(self.ble_channel_spacing * self.ble_channel))

    def get_ble_channel(self):
        return self.ble_channel

    def set_ble_channel(self, ble_channel):
        self.ble_channel = ble_channel
        self.set_freq(self.ble_base_freq+(self.ble_channel_spacing * self.ble_channel))

    def get_ble_base_freq(self):
        return self.ble_base_freq

    def set_ble_base_freq(self, ble_base_freq):
        self.ble_base_freq = ble_base_freq
        self.set_freq(self.ble_base_freq+(self.ble_channel_spacing * self.ble_channel))

    def get_squelch_threshold(self):
        return self.squelch_threshold

    def set_squelch_threshold(self, squelch_threshold):
        self.squelch_threshold = squelch_threshold
        self.analog_simple_squelch.set_threshold(self.squelch_threshold)

    def get_rf_gain(self):
        return self.rf_gain

    def set_rf_gain(self, rf_gain):
        self.rf_gain = rf_gain
        self.iio_pluto_source_0.set_params(int(self.freq), self.sample_rate, 2000000, True, True, True, 'manual', self.rf_gain, '', True)

    def get_lowpass_filter(self):
        return self.lowpass_filter

    def set_lowpass_filter(self, lowpass_filter):
        self.lowpass_filter = lowpass_filter
        self.freq_xlating_fir_filter_lp.set_taps(self.lowpass_filter)

    def get_gmsk_sps(self):
        return self.gmsk_sps

    def set_gmsk_sps(self, gmsk_sps):
        self.gmsk_sps = gmsk_sps

    def get_gmsk_omega_limit(self):
        return self.gmsk_omega_limit

    def set_gmsk_omega_limit(self, gmsk_omega_limit):
        self.gmsk_omega_limit = gmsk_omega_limit

    def get_gmsk_mu(self):
        return self.gmsk_mu

    def set_gmsk_mu(self, gmsk_mu):
        self.gmsk_mu = gmsk_mu

    def get_gmsk_gain_mu(self):
        return self.gmsk_gain_mu

    def set_gmsk_gain_mu(self, gmsk_gain_mu):
        self.gmsk_gain_mu = gmsk_gain_mu

    def get_freq_offset(self):
        return self.freq_offset

    def set_freq_offset(self, freq_offset):
        self.freq_offset = freq_offset
        self.freq_xlating_fir_filter_lp.set_center_freq(-self.freq_offset)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.iio_pluto_source_0.set_params(int(self.freq), self.sample_rate, 2000000, True, True, True, 'manual', self.rf_gain, '', True)





def main(top_block_cls=ble_sniffer_no_gui, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
