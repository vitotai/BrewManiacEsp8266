var BChart = {
    offset: 0,
    url: 'chart.php',

    reqdata: function() {
        var t = this;
        var PD = 'offset=' + t.offset;
        var xhr = new XMLHttpRequest();
        xhr.open('GET', t.url + '?' + PD);
        //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        //	xhr.setRequestHeader("Content-length", PD.length);

        xhr.responseType = 'arraybuffer';
        xhr.onload = function(e) {
            // response is unsigned 8 bit integer
            var data = new Uint8Array(this.response);
            if (data.length == 0) {
                //console.log("zero content");
                if (t.timer) clearInterval(t.timer);
                t.timer = null;
                setTimeout(function() {
                    t.reqdata();
                }, 3000);
                return;
            }
            t.chart.process(data);
            t.offset += data.length;
            if (t.timer == null) t.settimer();
        };
        //console.log(PD);
        xhr.send();
    },
    settimer: function() {
        var t = this;
        //console.log("start timer at "+ t.chart.interval);
        t.timer = setInterval(function() {
            t.reqdata();
        }, t.chart.interval * 1000);
    },
    init: function(id) {
        this.chart = new BrewChart(id);
    },
    timer: null,
    pull: function() {
        //	this.reqdata();
    },
    setCelius: function(ic) {
        this.chart.setCelius(ic);
    },
    setYLabel: function(lb) {
        this.chart.ylabel(lb);
    },
    start: function() {
        if (this.running) return;
        this.running = true;
        this.offset = 0;
        this.chart.clear();
        this.reqdata();
        $("#tchart").show();
    },
    stop: function() {
        if (!this.running) return;
        this.running = false;

        if (this.timer) {
            clearInterval(this.timer);
            this.timer = null;
        }
        $("#tchart").hide();
    }
};