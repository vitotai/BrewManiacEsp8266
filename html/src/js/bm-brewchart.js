/***************** Start of BrewChart *********************/

var AddHopEventId = 7;
var TemperatureReachedId = 1;
var ResumeBrewShortText = 'R';
var AddHopShortText='H';
var TemperatureReachedShortText='TR';
var StageLineColor= 'rgba(0, 45, 112,0.8)';
var StageTextColor= 'rgba(125,125,125,1)';
var LineLabels=["Time","Set","#1", "#2", "#3", "#4", "#5", "#6", "#7"];
var LineColors=['rgba(250, 0, 0, 1)','rgba(54, 162, 235, 1)',
'rgba(207, 56, 199, 1)',
'rgba(255, 206, 86, 1)',
'rgba(75, 192, 192, 1)',
'rgba(153, 102, 255, 1)',
'rgba(255, 159, 64, 1)'];

//function C2F(c) { return Math.round((c * 1.8 + 32) * 10) / 10; }
//function F2C(f) { return Math.round((f - 32) / 1.8 * 10) / 10; }

var BrewChart = function(div) {
    this.cid = div;
    this.ctime = 0;
    this.interval = 60;
    this.numLine = 1;
    this.lidx = 0;
    this.celius = true;
    this.spLabel = "set";
    this.sp = 35;
    this.LL = LineLabels;
    this.lineColors = LineColors;
    this.config = $.extend(true, {}, BrewChart.config);
    this.realtime = false;
    this.data=[];
    this.anno=[];
    this.stage=[];
};
BrewChart.prototype.clear = function() {
    var t = this;
    t.ctime = 0;
    t.numLine = 1;
    t.lidx = 0;
    this.data = [];
    this.anno=[];
    this.stage=[];
    if (typeof t.myLine !== "undefined") {
        t.myLine.destroy();
    }
};

BrewChart.prototype.setCelius = function(c) {
    this.celius = c;
    this.ylabel=STR.ChartLabel + '(' + (c ? "°C" : "°F") + ')';
};
BrewChart.prototype.lineLabel = function(l, lb) {
    if (typeof lb !== "undefined") {
        this.LL[l] = lb;
    } else {
        this.LL = l;
    }
};
BrewChart.prototype.formatTime = function(tm) {
    var d=new Date(tm);
    mm = d.getMinutes();
    hh = d.getHours();
    return ("" + ((hh > 9) ? hh : "0" + hh) + ':' + ((mm > 9) ? mm : "0" + mm));
};
BrewChart.prototype.startRecord = function() {
    //this.config.data.labels.push(this.ftime(this.ctime));
    this.record = [new Date(1000*(this.ctime+ this.starttime))] ;
};
BrewChart.prototype.addData = function(i, tp) {
    //	if(!this.celius && !isNaN(tp)) tp=C2F(tp);
    this.record[i+1]=tp;
};
BrewChart.prototype.finishRecord = function() {
    this.data.push(this.record);
    this.ctime += this.interval;
};

BrewChart.prototype.tempFormat = function(y) {
    var v = parseFloat(y);
    if (isNaN(v)) return "--";
    var DEG = this.celius ? "&deg;C" : "&deg;F";
    return parseFloat(v).toFixed(2) + DEG;
};

BrewChart.prototype.createChart = function() {
    var t = this;

    if(t.data.length ==0) return;

    var labeldiv = document.createElement("div");
    labeldiv.className = "hide";
    document.body.appendChild(labeldiv);
    var labels=[];
    var colors=[];
    for(var i=0;i<=t.numLine+1;i++){
        labels.push(t.LL[i]);
        colors.push(t.lineColors[i]);
    }

    var opt = {
        labels: labels,
        colors: colors,
        connectSeparatedPoints: true,
        ylabel: t.ylabel,
        axisLabelFontSize: 12,
        animatedZooms: true,
        width:'auto',
        gridLineColor: '#ccc',
        gridLineWidth: '0.1px',
        labelsDiv:  document.getElementById(t.div + "-label"),
        legend: 'follow',
//        labelsSeparateLines: true,
        labelsDivStyles: {
            'text-align': 'right'
        },
        displayAnnotations: true,
        //showRangeSelector: true,
        strokeWidth: 2,
        axes: {
            y: {
                valueFormatter: function(y) {
                    return t.tempFormat(y);
                }
            },                    
            x: {
                valueFormatter: function(v){
                    return t.formatTime(v);
                }
            }

        },
        highlightCircleSize: 2,
        highlightSeriesOpts: {
            strokeWidth: 1.5,
            strokeBorderWidth: 1,
            highlightCircleSize: 5
        },
/*        highlightCallback: function(e, x, pts, row) {
            t.showLegend(x, row);
        },
        unhighlightCallback: function(e) {
            t.hideLegend();
        },
*/
        underlayCallback: function(ctx, area, graph) {
                ctx.save();
                try {
                    t.drawBackground(ctx, area, graph);
                } finally {
                    ctx.restore();
                }
            }
    };
    t.chart = new Dygraph(document.getElementById(t.cid), t.data, opt);
    document.getElementById(t.cid).parentNode.onresize=function(){
        t.chart.resize();
    };
};

BrewChart.prototype.update = function() {
    if(typeof this.chart !="undefined"){
        this.chart.updateOptions({'file':this.data});
        this.chart.setAnnotations(this.anno);
    }else{
        this.createChart();
        this.chart.setAnnotations(this.anno);
    }
};
BrewChart.prototype.findStageBlocks = function(g, start, end) {
    "use strict";
    
    if(this.stage.length <1) return [];

    var result = [];
    var sidx=0;
    var timeStart = Math.floor(start / 1000) - this.starttime;;
    var timeEnd =Math.floor(end / 1000) - this.starttime;;

    while(this.stage[sidx].time < timeStart && sidx < this.stage.length) sidx++;
    var cstage=this.stage[sidx].stage;
    var ctext = this.stage[sidx].text;
    var start = timeStart;
    var i;
    for (i = sidx+1; i < this.stage.length 
                        && this.stage[i].time < timeEnd; i++) {
        result.push({
            stage:cstage,
            text:ctext,
            start:start,
            end:this.stage[i].time
        });
        cstage=this.stage[i].stage;
        ctext = this.stage[i].text;
        start = this.stage[i].time;
    }
        result.push({
            stage:cstage,
            text:ctext,
            start:start,
            end:timeEnd
        });
    

    return result;
};

BrewChart.prototype.drawBackground = function(ctx, area, graph) {
    var timeStart = graph.toDataXCoord(area.x);
    var timeEnd = graph.toDataXCoord(area.x + area.w);
    var blocks = this.findStageBlocks(graph, timeStart, timeEnd); // rowEnd is exclusive
    var period= (timeEnd - timeStart)/1000;
    var startX = area.x; // start drawing from 0 - the far left
    

    for (var i = 0; i < blocks.length; i++) {
        var block = blocks[i];

        var r = (block.end - block.start) /period; // as a fraction of the entire display
        var width = Math.floor(area.w * r);
        // text        
        var tm = ctx.measureText(block.text);

        if( width > tm.width * 0.6){
            var textLeft=startX + ((tm.width < width)? (width - tm.width)/2:0);        
            ctx.strokeStyle = StageTextColor;
            ctx.strokeText(block.text,textLeft , area.h/2 -(tm.actualBoundingBoxAscent + tm.actualBoundingBoxDescent), width);
        }else{
            // rotate text
            ctx.save();
            var x = startX + width / 2;
            var y = area.y + area.h / 2;
            ctx.translate(x, y);
            ctx.rotate(-Math.PI / 2);

            ctx.strokeText(block.text, 0 , (tm.actualBoundingBoxAscent + tm.actualBoundingBoxDescent)/2);

            ctx.restore();

        }
        ctx.strokeStyle = StageLineColor;
        ctx.beginPath();
//        ctx.setLineDash([1,1]);
//        ctx.lineWidth = 2;
//        ctx.lineCap = 'round';
        ctx.moveTo(startX, area.y);
        ctx.lineTo(startX, area.y + area.h);
        ctx.stroke();
        //ctx.strokeRect(startX, area.y, width, area.h);
        startX = startX + width;
    }
    
};
BrewChart.prototype.addStage = function(s) {
    var text;
    if (s < STR.stageName.length) {
        text = STR.stageName[s];
    } else if (s == 100) {
        text = STR.ManualMode;
    } else if (s == 103) {
        text = STR.PIDAutoTune;
    } else {
        return;        
    }
    
    this.stage.push({stage:s,time:this.ctime, text:text});
};
BrewChart.prototype.addEvent = function(s) {
    if (s == AddHopEventId || s == TemperatureReachedId) {
        var st = (s == AddHopEventId)? AddHopShortText:TemperatureReachedShortText;
        var text = STR.event[s];
        var bottom = (s == TemperatureReachedShortText);
        this.anno.push({
            series: this.LL[1],
            x:(this.starttime +this.ctime) *1000,
            shortText: st,
            text: text,
            attachAtBottom: bottom
        });    
    }
};
BrewChart.prototype.addResume = function(s) {
    while(this.record.length <= this.numLine +1) this.record.push(NaN);
    this.finishRecord();
    this.anno.push({
        series: this.LL[1],
        x:(this.starttime +this.ctime) *1000,
        shortText: ResumeBrewShortText,
        text: STR.ResumeBrew,
        attachAtBottom: true
    });
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
            t.createChart();
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
                t.startRecord(); // add one time interval
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
            if (++t.lidx >= t.numLine){
                t.lidx = 0;
                t.finishRecord();
            }
        }
    }
    t.update();
};
/***************** End of BrewChart *********************/
