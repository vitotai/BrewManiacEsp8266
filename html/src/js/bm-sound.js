function SoundBox(beep, uicb) {
    var b = this;
    b.beep = beep;
    beep.loop = true;
    b.meta = {};
    b.audioTimeout = null;
    b.playing = false;
    b.queue = [];
    b.onAudioLoaded = null;
    b.audioLoaded = false;
    //    b.uiNeeded = false;
    b.uicb = uicb;

    function loadAudio() {
        //var b=this;
        b.audio = new Audio(b.audiosrc);
        b.audio.oncanplaythrough = function() {
            //console.log("audio file downloaded");
            b.audio.oncanplaythrough = null;
            b.audioLoaded = true;
            if (b.onAudioLoaded !== null) b.onAudioLoaded();
        };
        b.audio.load();
    }

    function checkQueue() {
        //var b=this;
        if (b.queue.length == 0) {
            //console.log("empty queue");
            return;
        }
        if (b.playing) return;
        var target = b.queue.shift();
        //console.log("dequeue " + target.s);
        //b.playing = true;
        if (target.s == "beep") {
            //console.log("beep play");
            b.beep.currentTime = 0.01;
            var promise = b.beep.play();
            if (promise !== undefined) {
                promise.then(_ => {
                    // Autoplay started!
                    b.playing = true;
                }).catch(error => {
                    // Autoplay was prevented.
                    // Show a "Play" button so that user can start playback.
                    //console.log("UI needed to play");
                    //b.uiNeeded = true;
                    b.uicb();
                });
            }
            b.audioTimeout = null;
            b.beep.ontimeupdate = function() {
                //console.log("beep time:"+b.beep.currentTime);
                if (b.audioTimeout == null && b.beep.currentTime > 0.01) {
                    b.beep.ontimeupdate = null;
                    b.audioTimeout = setTimeout(function() {
                        b.beep.pause();
                        clearTimeout(b.audioTimeout);
                        b.playing = false;
                        checkQueue();
                    }, (target.t - b.beep.currentTime) * 1000);
                }
            };

        } else {
            var repeat = b.meta[target.s].r;
            var time = b.meta[target.s].s;
            var duration = b.meta[target.s].t;
            if (time == 0) time = 0.01;
            //console.log("Start: " + b.audio.seekable.start(0) + " End: "  + b.audio.seekable.end(0));
            //console.log("start:"+time + " duration:" +duration +" repeat"+repeat);

            try {
                b.audio.currentTime = time;
                //console.log("try play:" + b.audio.currentTime);
                if (b.audio.currentTime != time) console.log("error settime");

                var promise = b.audio.play();

                if (promise !== undefined) {
                    promise.then(_ => {
                        // Autoplay started!
                        b.playing = true;
                    }).catch(error => {
                        // Autoplay was prevented.
                        // Show a "Play" button so that user can start playback.
                        //console.log("UI needed to play");
                        //  b.uiNeeded = true;
                        b.uicb();
                    });
                }
            } catch (e) {
                //console.log("play exception:"+e);
                //console.log("catch play:" + b.audio.currentTime);

                var progress = function() {
                    b.audio.removeEventListener('progress', progress);
                    b.audio.currentTime = time;
                    b.audio.play();
                };
                b.audio.addEventListener('progress', progress, false);
                b.audio.play();
            }

            var played = 0;
            var end = time + duration;
            var st = Date.now();
            var TIMEOUT = 2000;

            b.audioTimeout = setInterval(function() {
                //console.log("current time:"+b.audio.currentTime);
                if ((Date.now() - st) > TIMEOUT) {
                    // somehow the play doesn't start.
                    if (b.audio.currentTime == time) {
                        console.log("error not playing");
                        b.audio.pause();
                        b.audio = new Audio(b.audiosrc);
                        b.audioLoaded = false;
                        b.audio.oncanplaythrough = function() {
                            //console.log("audio file downloaded");
                            b.audio.oncanplaythrough = null;
                            b.audioLoaded = true;
                        };
                        b.audio.load();
                        // play beep instead
                        /*                        clearInterval(b.audioTimeout);
                                                b.playing = false;
                                                b.queue.unshift({ s: "beep", t: b.beep.duration });
                                                checkQueue(); */
                    }
                }
                if (b.audio.currentTime > end) {
                    b.audio.pause();
                    played++;
                    if (played < repeat) {
                        b.audio.currentTime = time;
                        b.audio.play();
                        st = Date.now();
                    } else {
                        clearInterval(b.audioTimeout);
                        b.playing = false;
                        checkQueue();
                    }
                }
            }, 100);


            /*
                    b.audio.addEventListener('timeupdate', function(ev) {
                      if (b.audio.currentTime > end) {
                            played++;
                            b.audio.pause();
                            if(played < repeat){
                                b.audio.currentTime=time;
                                b.audio.play();
                            }else{
                                b.playing=false;
                                b.checkQueue();
                            }
                        }
                    },false);
                    
                    
                    b.audioTimeout=setInterval(function(){
                        played++;
                        b.audio.pause();
                        if(played < repeat){
                            b.audio.currentTime=time;
                            b.audio.play();
                        }else{
                            clearInterval(b.audioTimeout);
                            b.playing=false;
                            b.checkQueue();
                        }
                   },1000 * duration);
            */
        }
    }

    function playAudio(sid) {
        b.queue.push({ s: sid });
        //console.log("enqueue " + sid);
        checkQueue();
    }

    function playBeep(repeat) {
        //var b=this;
        var time = b.beep.duration;
        if (repeat) time = time * 5;
        b.queue.push({ s: "beep", t: time });
        checkQueue();
    }

    b.play = function(sid, mul) {
        //if (b.uiNeeded) return;
        loop = (typeof mul == "undefined") ? false : mul;
        //var b=this;
        //console.log("Play " + sid);
        if (b.audioLoaded && (typeof b.meta[sid] != "undefined" && b.meta[sid].t > 0)) {
            playAudio(sid);
        } else {
            playBeep(loop);
        }
    };
    b.load = function(json) {
        //console.log(json);
        b.audiosrc = json.src;
        b.meta = json.seg;
        loadAudio();
    };
}

var Sound = {
    audiocfg: "sounds.json",
    loadJson: function() {
        var b = this;
        $.ajax({
            url: b.audiocfg,
            type: "GET",
            dataType: "json",
            success: function(json) {
                b.sb.load(json);
            },
            error: function(e) {
                console.log("sound meta not found");
            }
        });
    },
    play: function(sid) {
        this.sb.play(sid);
    },
    init: function(cb) {
        var t = this;
        $("#audioplay").hide();

        t.sb = new SoundBox(Beep, function() {
            console.log("User interaction is needed.");
            $("#audioplay").show();
        });

        $("#audioplay button").button({ text: false, icons: { primary: "ui-icon-volume-on" } }).click(function() {
            $("#audioplay").hide();
            t.sb.play("open");
        });
        t.sb.onAudioLoaded = function() {
            t.sb.play("open");
        };
        setTimeout(function() {
            t.loadJson();
        }, 500);
    }
};