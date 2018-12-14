/***************** Start of BrewChart *********************/
function C2F(c) { return Math.round((c * 1.8 + 32) * 10) / 10; }

function F2C(f) { return Math.round((f - 32) / 1.8 * 10) / 10; }

var BrewChart = function(div) {
    this.cid = div;
    this.ctime = 0;
    this.interval = 60;
    this.numLine = 1;
    this.lidx = 0;
    this.celius = true;
    this.spLabel = "set";
    this.sp = 35;
    this.LL = ["#1", "#2", "#3", "#4", "#5", "#6", "#7"];
    this.config = $.extend(true, {}, BrewChart.config);
    this.realtime = false;
};
BrewChart.prototype.clear = function() {
    var t = this;
    t.ctime = 0;
    t.numLine = 1;
    t.lidx = 0;
    this.config = $.extend(true, {}, BrewChart.config);
    if (typeof t.myLine !== "undefined") {
        t.myLine.destroy();
    }
};
BrewChart.config = {
    type: 'line',
    data: {
        labels: [],
        datasets: []
    },
    options: {
        responsive: true,
        legend: { position: 'bottom' },
        hover: { mode: 'label' },
        scales: {
            xAxes: [{
                display: true,
                scaleLabel: { display: false }
            }],
            yAxes: [{
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'Temperature'
                }
            }]
        },
        title: { display: false },
        annotation: {
            drawTime: "afterDraw",
            annotations: []
        }
    }
};
BrewChart.spcolor = 'rgba(250, 0, 0, 0.45)';
BrewChart.bgcolor = ['rgba(54, 162, 235, 0.8)',
    'rgba(255, 99, 132, 0.8)',
    'rgba(255, 206, 86, 0.8)',
    'rgba(75, 192, 192, 0.8)',
    'rgba(153, 102, 255, 0.8)',
    'rgba(255, 159, 64, 0.8)'
];
BrewChart.eventAnno = {
    dt: 'e',
    type: 'line',
    mode: 'vertical',
    scaleID: 'x-axis-0',
    //		value: '00:00',
    borderColor: '#42f445',
    borderWidth: 2,
    label: {
        //			text:'',
        color: '#00215b',
        borderWidth: 0,
        anchor: 'top'
    }
};
BrewChart.stageAnno = {
    dt: 's',
    type: 'line',
    mode: 'vertical',
    scaleID: 'x-axis-0',
    //		value: '00:00',
    borderColor: '#8942f4',
    borderWidth: 2,
    label: {
        //			text:'',
        color: '#00215b',
        borderWidth: 0,
        anchor: 'top'
    }
};
BrewChart.resumeAnno = {
    dt: 's',
    type: 'line',
    mode: 'vertical',
    scaleID: 'x-axis-0',
    //		value: '00:00',
    borderColor: 'black',
    borderWidth: 2,
    borderDash: [2, 2],
    label: {
        //			text:'',
        color: 'red',
        borderWidth: 0,
        anchor: 'top'
    }
};

BrewChart.prototype.ylabel = function(l) {
    this.config.options.scales.yAxes[0].scaleLabel.labelString = l;
};
BrewChart.prototype.setCelius = function(c) {
    this.celius = c;
    this.ylabel(STR.ChartLabel + '(' + (c ? "°C" : "°F") + ')');
};
BrewChart.prototype.lineLabel = function(l, lb) {
    if (typeof lb !== "undefined") {
        this.LL[l] = lb;
    } else {
        this.LL = l;
    }
};
BrewChart.prototype.ftime = function(tm) {
    var mm;
    var hh;
    if (this.realtime) {
        var utime = this.starttime + tm;
        var d = new Date(utime * 1000);
        mm = d.getMinutes();
        hh = d.getHours();
    } else {
        mm = Math.floor(tm / 60);
        hh = Math.floor(mm / 60);
        mm = mm - hh * 60;
    }
    return ("" + ((hh > 9) ? hh : "0" + hh) + ':' + ((mm > 9) ? mm : "0" + mm));
};
BrewChart.prototype.incTime = function() {
    // format time, use hour and minute only.
    this.config.data.labels.push(this.ftime(this.ctime));
    this.ctime += this.interval;
    //	console.log("incTime:"+ this.ctime/this.interval);
};
BrewChart.prototype.addData = function(i, tp) {
    //	if(!this.celius && !isNaN(tp)) tp=C2F(tp);

    this.config.data.datasets[i].data.push(tp);
};
BrewChart.prototype.chart = function() {
    var t = this;
    var ctx = document.getElementById(t.cid).getContext("2d");

    var spl = {
        label: t.spLabel,
        data: [],
        fill: false,
        borderColor: BrewChart.spcolor,
        backgroundColor: BrewChart.spcolor,
        pointBorderColor: BrewChart.spcolor,
        pointBackgroundColor: BrewChart.spcolor,
        borderDash: [8, 4],
        pointBorderWidth: 1,
        pointRadius: 1,
        pointStyle: 'dash',
        lineTension: 0,
        cubicInterpolationMode: 'linear'
    };
    t.config.data.datasets.push(spl);
    for (i = 0; i < t.numLine; i++) {
        var ds = {
            label: t.LL[i],
            data: [],
            fill: false
        };
        var bgcolor = BrewChart.bgcolor[i];
        ds.borderColor = bgcolor;
        ds.backgroundColor = bgcolor;
        ds.pointBorderColor = bgcolor;
        ds.pointBackgroundColor = bgcolor;
        ds.pointBorderWidth = 1;
        ds.pointRadius = 1;
        ds.lineTension = 0;
        ds.pointStyle = 'line';
        ds.cubicInterpolationMode = 'linear';

        t.config.data.datasets.push(ds);
    }
    //if(t.numLine ==1) t.config.options.legend.display=false;
    t.myLine = new Chart(ctx, t.config);
};
BrewChart.prototype.addStage = function(s) {

    var annos = this.config.options.annotation.annotations;
    var time = this.ftime(this.ctime);
    if (annos.length > 0 &&
        annos[annos.length - 1].value == time) {
        if (annos[annos.length - 1].dt == 'e') {
            // remove the event, stage takes priority
            //console.log("duplicated@"+ time + " removed:" +annos[annos.length-1].label.text);
            annos.pop();
        }
    }

    var ano = $.extend(true, {}, BrewChart.stageAnno);
    if (s < STR.stageName.length) {
        ano.label.text = STR.stageName[s];
    } else if (s == 100) {
        ano.label.text = STR.ManualMode;
    } else if (s == 103) {
        ano.label.text = STR.PIDAutoTune;
    } else {
        return;
    }
    ano.value = time;
    this.config.options.annotation.annotations.push(ano);
};
BrewChart.prototype.addEvent = function(s) {
    if (s == 1 || s == 10) {
        var annos = this.config.options.annotation.annotations;
        var time = this.ftime(this.ctime);
        if (annos.length > 0 &&
            annos[annos.length - 1].value == time) {
            // ignore this
            //console.log("duplicated@"+ time +" for " + STR.event[s] + " already:"+annos[annos.length-1].label.text);
            return;
        }
        // temperature reach & boil end
        var ano = $.extend(true, {}, BrewChart.eventAnno);
        ano.label.text = STR.event[s];
        ano.value = time;
        this.config.options.annotation.annotations.push(ano);
    }
};
BrewChart.prototype.addResume = function(s) {
    this.incTime();
    for (i = 0; i < this.numLine; i++) this.addData(i, NaN);

    var ano = $.extend(true, {}, BrewChart.resumeAnno);
    if (this.realtime) this.ctime += s * 60;
    ano.value = this.ftime(this.ctime);
    ano.label.text = STR.ResumeBrew;
    this.config.options.annotation.annotations.push(ano);
};

BrewChart.testData = function(data) {
    if (data[0] != 0xFF) return false;
    var s = data[1] & 0x07;
    if (s > 5) return false;

    return { sensor: s, f: data[1] & 0x10 };
};

BrewChart.prototype.process = function(data) {
    var t = this;
    for (var i = 0; i < data.length;) {
        var d0 = data[i++];
        var d1 = data[i++];
        if (d0 == 0xFF) { // header. 
            //console.log(""+t.ctime/t.interval +" header");
            t.numLine = d1 & 0x07;
            var p = data[i++];
            p = p * 256 + data[i++];
            t.interval = p;
            // 
            t.starttime = (data[i] << 24) + (data[i + 1] << 16) + (data[i + 2] << 8) + data[i + 3];
            i += 4;
            t.chart();
        } else if (d0 == 0xF1) { // stage
            //console.log(""+t.ctime/t.interval +" Stage:"+d1);
            t.addStage(d1);
        } else if (d0 == 0xF2) { // event
            //console.log(""+t.ctime/t.interval +": Event:"+d1);
            t.addEvent(d1);
        } else if (d0 == 0xF3) { // setpoint
            var hh = data[i++];
            var ll = data[i++];
            t.sp = ((hh & 0x7F) * 256 + ll) / 100;
        } else if (d0 == 0xFE) { // resume
            t.addResume(d1);
        } else if (d0 < 128) { // time.
            if (t.lidx == 0) {
                t.incTime(); // add one time interval
                t.addData(0, t.sp);
            }
            var tp = d0 * 256 + d1;
            if (tp == 0x7FFF) { //(tp == 0x7FFF){ // invalid data, use latest data
                t.addData(t.lidx + 1, NaN);
            } else {
                var val = tp / 100;
                if ((t.celius && val > 120) || val > 238) t.addData(t.lidx + 1, NaN);
                else t.addData(t.lidx + 1, val);
            }
            if (++t.lidx >= t.numLine) t.lidx = 0;
        }
    }
    t.myLine.update();
};
/***************** End of BrewChart *********************/
