BrewChart.prototype.vstart = function(numline) {
    var t = this;
    //console.log(""+t.ctime/t.interval +" header");
    t.numLine = numline;
    t.interval = 5;
            // 
    t.starttime = Math.round((new Date()).getTime()/1000);
    t.createChart();
};

BrewChart.prototype.vadd = function(data) {
    this.data.push(data);
    this.update();
};

var ChartData = {
    offset: 0,
    volatile:false,
    reqdata: function() {
        var t = this;

        t.bm.reqdata(t.offset,function(data) {
            if (data.length == 0) {
                //console.log("zero content");
                if (t.timer) clearInterval(t.timer);
                t.timer = null;
                setTimeout(function() {
                    t.reqdata();
                }, 5000);
                return;
            }
            t.chart.process(data);
            t.offset += data.length;
            if (t.timer == null) t.settimer();
        });
    },
    settimer: function() {
        var t = this;
        //console.log("start timer at "+ t.chart.interval);
        t.timer = setInterval(function() {
            t.reqdata();
        }, t.chart.interval * 1000);
    },
    init: function(id,bm) {
        this.chart = new BrewChart(id);
        this.bm=bm;
    },
    timer: null,
    pull: function() {
        //this.reqdata();
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
        this.volatile = false;
        this.offset = 0;
        this.chart.clear();
        this.reqdata();
    },
    stop: function() {
        if (!this.running) return;
        this.running = false;

        if (this.timer) {
            clearInterval(this.timer);
            this.timer = null;
        }
    },
    vstart:function(numline){
        if (this.running) return;
        this.running = true;
        this.volatile = true;

        this.chart.clear();
        this.chart.vstart(numline);
    },
    vadd:function(data){
        this.chart.vadd(data);
    },
    isVolatileRunning:function(){
        return this.running && this.volatile;
    },
    clear:function(){
        this.chart.clear();
    },
    addTemp:function(temps){
        temps.unshift(new Date());
        this.chart.vadd(temps);
    }
};