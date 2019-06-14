      // settings
        var EEPROM = {
            "s_kp": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_ki": {
                max: 155,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kd": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kp2": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_ki2": {
                max: 155,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kd2": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kpall": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kiall": {
                max: 155,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_kdall": {
                max: 100,
                min: -100,
                inc: 1,
                decode: function(v) {
                    return v - 100;
                },
                encode: function(v) {
                    return v + 100;
                }
            },
            "s_sample_time": {
                max: 8000,
                min: 1500,
                inc: 250,
                decode: function(v) {
                    return v * 250;
                },
                encode: function(v) {
                    return Math.round(v / 250);
                }
            },
            "s_window": {
                max: 40000,
                min: 4000,
                inc: 250,
                decode: function(v) {
                    return v * 250;
                },
                encode: function(v) {
                    return Math.round(v / 250);
                }
            },
            "s_pwm": {
                max: 100,
                min: 0,
                inc: 1
            },
            "s_cal": {
                max: 5,
                min: -5,
                inc: 0.1,
                decode: function(v) {
                    return (v - 50) / 10;
                },
                encode: function(v) {
                    return v * 10 + 50;
                }
            },
            "s_pidstart": {
                max: 3.5,
                min: 1.0,
                inc: 0.1,
                decode: function(v) {
                    return (v) / 10;
                },
                encode: function(v) {
                    return v * 10;
                }
            },
            "s_piddoughin": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_unit": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["&deg;C", "&deg;F"],
            },
            "s_nodelay": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_boil": {
                max: 120,
                min: 80,
                inc: 1
            },
            "s_pumpcycle": {
                max: 15,
                min: 5,
                inc: 1
            },
            "s_pumprest": {
                max: 5,
                min: 0,
                inc: 1
            },
            "s_pumppremash": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_pumpmash": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_pumpmashout": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_pumpboil": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_pumpstop": {
                max: 120,
                min: 80,
                inc: 1
            },
            "s_pipe": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_skipadd": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_skipremove": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_skipiodine": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_iodine": {
                max: 120,
                min: 0,
                inc: 1
            },
            "s_whirlpool": {
                max: 2,
                min: 0,
                labels: ["<%= set_off %>", "<%= whirlpool_cold %>", "<%= whirlpool_hot %>"]
            },
            "s_hop": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_no %>", "<%= set_yes %>"]
            },
            "s_spenable": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_sptempctrl": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_spsensor": {
                max: 5,
                min: 0,
                inc: 1
            },
            "s_sptemp": {
                max: 80,
                min: 75,
                inc: 1
            },
            "s_spdiff": {
                max: 2.0,
                min: 0.5,
                inc: 0.1,
                decode: function(v) {
                    return (v) / 10;
                },
                encode: function(v) {
                    return v * 10;
                }
            },
            "s_btnbuzz": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_pprime": {
                max: 10,
                min: 0,
                inc: 1
            },
            "s_ppon": {
                max: 10000,
                min: 250,
                inc: 250,
                decode: function(v) {
                    return v * 250;
                },
                encode: function(v) {
                    return Math.round(v / 250);
                }
            },
            "s_ppoff": {
                max: 10000,
                min: 0,
                inc: 250,
                decode: function(v) {
                    return v * 250;
                },
                encode: function(v) {
                    return Math.round(v / 250);
                }
            },
            "s_preheat": {
                max: 3,
                min: 1,
                inc: 1,
                labels: ["<%= invalid %>", "<%= primary_heater %>", "<%= secondary_heater %>", "<%= both_heaters %>"]
            },
            "s_mashheat": {
                max: 3,
                min: 1,
                inc: 1,
                labels: ["<%= invalid %>", "<%= primary_heater %>", "<%= secondary_heater %>", "<%= both_heaters %>"]
            },
            "s_boilheat": {
                max: 3,
                min: 1,
                inc: 1,
                labels: ["<%= invalid %>", "<%= primary_heater %>", "<%= secondary_heater %>", "<%= both_heaters %>"]
            },
            "s_pbheat": {
                max: 3,
                min: 1,
                inc: 1,
                labels: ["<%= invalid %>", "<%= primary_heater %>", "<%= secondary_heater %>", "<%= both_heaters %>"]
            },
            "s_wlv": {
                max: 1,
                min: 0,
                inc: 1,
                labels: ["<%= set_off %>", "<%= set_on %>"]
            },
            "s_wlvtrigger": {
                max: 1000,
                min: 50,
                inc: 50,
                decode: function(v) {
                    return v * 50;
                },
                encode: function(v) {
                    return Math.round(v / 50);
                }
            },
            "s_pmpminrest": {
                max: 90,
                min: 2,
                inc: 1
            }
        };