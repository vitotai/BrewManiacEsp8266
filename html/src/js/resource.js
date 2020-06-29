       // strings, for localization
        var STR = {
            ChartLabel: "<%= temperature_chart_label %>",
            AddMalt: "<%= add_malt %>",
            RemoveMalt: "<%= remove_malt %>",
            Boil: "<%= boil %>",
            HopN: "<%= hop_num %>",
            PBHN: "<%= postboilhop_num %>",
            HopStandSession: "<%= hopstand_session %>",
            Cooling: "<%= cooling %>",
            Automation: "<%= automation %>",
            PIDAutoTune: "<%= pid_autotune %>",
            DelayStart: "<%= wait_to_start_automation %>",
            PumpRest: "<%= pump_rest_label %>",
            Idle: "<%= idle_mode_title %>",
            ManualMode: "<%= manual_mode %>",
            Setup: "<%= setup %>",
            PaddleLed: "<%= led_label_paddle %>",
            Unknown: "<%= unknown_state %>",
            Pause: "<%= paused %>",
            min: "<%= minute %>",
            Mash: "<%= mash_step %>",

            BoilEnd: "<%= boil_finish %>",
            EndHopStand: "<%= hopstand_finish %>",
            ResumeBrew: "<%= brew_resume %>",
            spargeSensor: "<%= spargesensor_label %>",
            stageName: ["<%= mash_in %>", "<%= mash_step1 %>", "<%= mash_step2 %>", "<%= mash_step3 %>", "<%= mash_step4 %>", "<%= mash_step5 %>", "<%= mash_step6 %>", "<%= mash_out %>", "<%= stage_boiling %>", "<%= stage_cooling %>", "<%= stage_whirlpool %>", "<%= stage_chilling %>", "<%= stage_hopstand %>"],

            errorWrongHopSchedule: "<%= errorWrongHopSchedule %>",
            errorMashoutTimeZero: "<%= errorMashoutTimeZero %>",
            errorHopStandSession: "<%= errorHopStandSession %>",
            errorPostHopSchedule: "<%= errorPostHopSchedule %>",
            errorTooManyHop: "<%= errorTooManyHop %>",
            errorTooManyHopStand: "<%= errorTooManyHopStand %>",
            errorTooManyPostBoilHop: "<%= errorTooManyPostBoilHop %>",
            errorWrongMashTemp: "Error Mash temp",

            DistillPreheat: "<%= distill_preheat %>",
            DistillHead: "<%= distill_head %>",
            DistillHeart: "<%= distill_heart %>",
            DistillTail: "<%= distill_tail %>",

            event: {1:"<%= temperature_reached %>",
                2:"<%= add_malt %>",
                3:"<%= remove_malt %>",
                4:"<%= iodine_test %>",
                5:"<%= pause %>",
                6:"<%= resume %>",
                7:"<%= add_hop %>",
                8:"<%= pwm_off %>",
                9:"<%= pwm_off %>",
                10:"<%= boil_finish %>",
                11:"<%= pump_rest_label %>",
                12:"Pump rest end",
                99:"<%= brew_finish %>"
            }
        };

        var STR_paddle = {
            PumpRest: "<%= paddle_rest_label %>",
        };
        var Beep = new Audio(
            "data:audio/mp4;base64,AAAAGGZ0eXBtcDQyAAAAAG1wNDJpc29tAAAAiGZyZWUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA8NtZGF03gQAAGxpYmZhYWMgMS4yOAAAAbiDMMg8nh2/D2/T8ev69XTq8e/s37d/ATKf/fSbKoypkqRJUyvG/x17c/08/yADKPhd2ABnXvPVKACM/i+mACGhfx/NcoAKjwvSpACcOo9tnWIAruv7FHGrqQAAAAAAAAAAAADbh86N7WvKD64KAAABuKn+328O0ZSsAAA+Hr61RgAAdQ5f3dHswAAAAXdXTymCcLAAAAAAAKKvDp/eAOJ/nAv5DxoAAAAAALAP7dcYzex45kWN3QoAAAANFtBGgbhwARKf/TMZKEiJMuTIlmfM8fnv/T41n/b4/wAa6c/439ZBqISDC5tLfzOthfpXgP0tLBPbaz9LzLvp8N4XveL6d4DmBtc9sBzXhaRFrnttZzFzPeChSfnL9HvSXz0fp0n1WD9Hk/aIPlNH8tJ+SwZwMYzQy9817b0BnGZKBcKBFE891K5CihVNPU1912OPqLEiS/VaurqGQkd4DLvwOP6oDp9ABh/YB5NUDW/DA4/qgOn0Hwg9WIsJB8ENUPVA9WIsJApEACATkAKhYcABItQJKhIxhoLBIIjUInMIisIhMIjcZUZuv2+pW99+Pt9/PWtfb8dXnj6r54+f6/vn+ff9R5O6NFPx8eK3c8I8fjMPfP2gfDYd9x8afDYHviEfDAn3IH+GB33N1/+qCCGNcy+DUk/bBwMGT4MmX+BjHmymHCAHpfxBXGJQU4mAIiH6gtcmCKoA7wBkABHCAAAAAAAGDfW0Z0UUUAAAcZgAAHyICCoAAKZgAABiHAEaVBAkQwkNQkEQkERoGwkESGETmERSEQqERtW3WH9v53vOe/X59vv1fH6fjzJ8/fPnr1/f/Ffvv6HOi2dnV1m2/b7epvnbU841Pay359YpvsHokSvD57Q64z5/0P8aD5/N7Q/X4AADS8EsPn84zNvzJfGL58xoG0PKf02hoLWHQtAXuHhBzEQF4gHeAKWAD5RAAn/Sc0AJdAAAAAK9AEbgAABKQFwACQ4BIp/9ypcq3MlSZFSVIljK378/z11n+vt+oAPl9Gr1oLA//erf1AAdX9/pbwvk5VdRn/n5yFXdIifs5933BXO9zqMfx7L7wr3N9xHLauHzIyD/+GJ/pcD/5pP3GDy2EEABm68tU3MOESx9Gd9bEgDV1e2bI/P9ZzeUGuW+9ap//7Ywa9fE/YoUbcx/EAF/IeFA6r52By/qQHU9wBe/uIAAAHG6FD2TAwOjYQO0EjUbARgAAAAEg2hYKj42gAADgADIwIAjgAAAAoRtb292AAAAbG12aGQAAAAA1Wi/F9VovxcAAB9AAAAYAAABAAABAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAGGlvZHMAAAAAEICAgAcAT///KP//AAAB+HRyYWsAAABcdGtoZAAAAAHVaL8X1Wi/FwAAAAEAAAAAAAAYAAAAAAAAAAAAAAAAAAEAAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAZRtZGlhAAAAIG1kaGQAAAAA1Wi/F9VovxcAAB9AAAAYAFXEAAAAAAAhaGRscgAAAAAAAAAAc291bgAAAAAAAAAAAAAAAAAAAAFLbWluZgAAABBzbWhkAAAAAAAAAAAAAAAkZGluZgAAABxkcmVmAAAAAAAAAAEAAAAMdXJsIAAAAAEAAAEPc3RibAAAAGdzdHNkAAAAAAAAAAEAAABXbXA0YQAAAAAAAAABAAAAAAAAAAAAAgAQAAAAAB9AAAAAAAAzZXNkcwAAAAADgICAIgAAAASAgIAUQBUAAMsAAAAAAAAm3AWAgIACFYgGgICAAQIAAAAgc3R0cwAAAAAAAAACAAAAAQAAAAAAAAAGAAAEAAAAADBzdHN6AAAAAAAAAAAAAAAHAAAAKgAAAKsAAADLAAAArQAAAKkAAAC/AAAABgAAABxzdHNjAAAAAAAAAAEAAAABAAAABwAAAAEAAAAUc3RjbwAAAAAAAAABAAAAqAAAACBjdHRzAAAAAAAAAAIAAAAB///8AAAAAAYAAAAAAAAAiGZyZWUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
        );

        var ButtonLabels = {
            0: {
                u: "",
                d: "",
                s: "<%= btn_quit %>",
                e: "<%= btn_go %>",
                i: ""
            },
            1: {
                u: "",
                d: "",
                s: "",
                e: "<%= btn_ok %>",
                i: ""
            },
            2: {
                u: "",
                d: "",
                s: "<%= btn_edit %>",
                e: "<%= btn_ok %>",
                i: ""
            },
            3: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_arrow %>",
                e: "",
                i: ""
            },
            4: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_done %>",
                e: "",
                i: ""
            },
            5: {
                u: "",
                d: "",
                s: "<%= btn_no %>",
                e: "<%= btn_yes %>",
                i: ""
            },
            6: {
                u: "",
                d: "<%= btn_manual %>",
                s: "<%= btn_auto %>",
                e: "<%= btn_setup %>",
                i: ""
            },
            7: {
                u: "",
                d: "",
                s: "",
                e: "",
                i: "<%= btn_i_pump_rest %>"
            },
            8: {
                u: "",
                d: "",
                s: "<%= btn_yes %>",
                e: "<%= btn_pump %>",
                i: "<%= btn_i_continue_q %>"
            },
            9: {
                u: "",
                d: "",
                s: "<%= btn_yes %>",
                e: "<%= btn_no %>",
                i: "<%= btn_i_continue_q %>"
            },
            10: {
                u: "",
                d: "",
                s: "",
                e: "<%= btn_pump %>",
                i: ""
            },
            11: {
                u: "",
                d: "",
                s: "<%= btn_time %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            12: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "",
                e: "<%= btn_pump %>",
                i: ""
            },
            13: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "",
                e: "",
                i: ""
            },
            14: {
                u: "",
                d: "",
                s: "<%= btn_exit %>",
                e: "",
                i: ""
            },
            15: {
                u: "",
                d: "",
                s: "<%= btn_ok %>",
                e: "",
                i: ""
            },
            16: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_end %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            17: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_heat %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            18: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_pause %>",
                e: "<%= btn_stp %>",
                i: ""
            },
            19: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_pump_pause %>",
                e: "<%= btn_stp %>",
                i: ""
            },
            20: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_pause %>",
                e: "",
                i: ""
            },
            21: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_skip %>",
                e: "<%= btn_ok %>",
                i: ""
            },
            22: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "",
                e: "<%= btn_ok %>",
                i: ""
            },
            23: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_quit %>",
                e: "<%= btn_ok %>",
                i: ""
            },
            24: {
                u: "",
                d: "<%= btn_down %>",
                s: "<%= btn_quit %>",
                e: "<%= btn_ok %>",
                i: ""
            },
            25: {
                u: "<%= btn_up %>",
                d: "",
                s: "<%= btn_quit %>",
                e: "<%= btn_ok %>",
                i: ""
            },
            26: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_pause %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            27: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_run %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            28: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_done %>",
                e: "<%= btn_more %>",
                i: ""
            },
            29: {
                u: "<%= btn_up %>",
                d: "",
                s: "",
                e: "<%= btn_ok %>",
                i: ""
            },
            30: {
                u: "",
                d: "<%= btn_down %>",
                s: "",
                e: "<%= btn_ok %>",
                i: ""
            },
            31: {
                u: "",
                d: "",
                s: "<%= btn_exit %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            32: {
                u: "",
                d: "",
                s: "<%= btn_no %>",
                e: "<%= btn_yes %>",
                i: "<%= btn_i_auto_tune_pid_q %>"
            },
            33: {
                u: "",
                d: "",
                s: "<%= btn_no %>",
                e: "<%= btn_yes %>",
                i: "<%= btn_i_stop_autotune_q %>"
            },
            34: {
                u: "",
                d: "",
                s: "",
                e: "<%= btn_yes %>",
                i: "<%= btn_i_run_ap_mode %>"
            },
            35: {
                u: "<%= btn_extend %>",
                d: "",
                s: "<%= btn_skip %>",
                e: "<%= btn_back %>",
                i: ""
            },
            36: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_skip %>",
                e: "<%= btn_pump %>",
                i: ""
            },
            37: {
                u: "",
                d: "",
                s: "<%= btn_mashout %>",
                e: "<%= btn_extend %>",
                i: ""
            },
            38: {
                u: "",
                d: "<%= btn_manual %>",
                s: "<%= btn_auto %>",
                e: "<%= btn_cancel %>",
                i: ""
            }
        };

        var ButtonLabels_paddle = {
            7: {
                u: "",
                d: "",
                s: "",
                e: "",
                i: "<%= btn_i_stir_rest %>"
            },
            8: {
                u: "",
                d: "",
                s: "<%= btn_yes %>",
                e: "<%= btn_stir %>",
                i: "<%= btn_i_continue_q %>"
            },
            10: {
                u: "",
                d: "",
                s: "",
                e: "<%= btn_stir %>",
                i: ""
            },
            11: {
                u: "",
                d: "",
                s: "<%= btn_time %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            12: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "",
                e: "<%= btn_stir %>",
                i: ""
            },
            16: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_end %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            17: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_heat %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            19: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_stir_pause %>",
                e: "<%= btn_stp %>",
                i: ""
            },
            26: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_pause %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            27: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_run %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            31: {
                u: "",
                d: "",
                s: "<%= btn_exit %>",
                e: "<%= btn_stir %>",
                i: ""
            },
            36: {
                u: "<%= btn_up %>",
                d: "<%= btn_down %>",
                s: "<%= btn_skip %>",
                e: "<%= btn_stir %>",
                i: ""
            }
        };

        var ButtonLabels_distill = {
            6: {
                u: "<%= btn_distill %>",
                d: "<%= btn_manual %>",
                s: "<%= btn_auto %>",
                e: "<%= btn_setup %>",
                i: ""
            }
        };

        var Unit = {
            liter: "<%= liter_label %>",
            gal: "<%= gallon_label %>",
            lb: "<%= lb_label %>",
            kg: "<%= kg_label %>",
            min: "<%= minute %>",
            oz: "<%= oz_label %>",
            qt: "<%= quart_label %>",
            gram: "<%= gram_label %>",
            degf: "&#8457;", //"°F",
            degc: "&#8451;", //"°C",
            Lovibond: "L",
            plato: "&deg;P"
        };

        var Hint={
            S:"<%= hint_setting %>", // setup
            U:"<%= hint_disconnected %>", //unknown, disconnection
            T:"<%= hint_autotune %>", // Auto tune
            I:"<%= hint_idle %>", // Idle mode
            M:"<%= hint_manual %>",
            D:"<%= hint_distill %>",
        };