// JavaScript source code


//var Module;
//if (!Module) Module = (typeof Module !== 'undefined' ? Module : null) || {};

Module["isSupportViewBoxPlayer"] = function () {

    var i32Buf = new Int32Array([259]);
    var i8Buf = new Int8Array(i32Buf.buffer);
    if (i8Buf[0] != 3)
        return false;

    //try {
    //    var tempmediaSource = new MediaSource;
    //    var tempurl = URL.createObjectURL(tempmediaSource);
    //    tempmediaSource = null;
    //    tempurl = null;
    //} catch (err) {
    //    console.error(err);
    //    return false;
    //}
    //return true;

    return window.MediaSource && window.MediaSource.isTypeSupported('video/mp4; codecs="avc1.42E01E,mp4a.40.2"');
}


Module["createViewBoxPlayer"] = function (para) {

    function ViewBoxPlayer(pp) {

        var video_element = pp.wrap;
        var video_url = "http://" + location.host + pp.videouri;
        var videosize;
        var reqsize;
        var rangestartpos;
        var endoffile = false;
        var xhr;
        var initlen;
        var persecondlen;
        var recvbuf;
        var recvlen;
        var addbuffer = Module.cwrap("_Z11testSupportPci", 'number', ['array', 'number']);
        var getinitseg = Module.cwrap("_Z10GetInitSegPci", 'number', ['number', 'number']);
        var getdatapersecond = Module.cwrap("_Z20GetDataRatePerSecondv", 'number', []);
        var getnormalSeg = Module.cwrap("_Z12GetNormalSegPci", 'number', ['number', 'number']);
        var getlastnormalSeg = Module.cwrap("_Z16GetLastNormalSegPci", 'number', ['number', 'number']);
        var getnormalSegDuration = Module.cwrap("_Z14GetSegDurationv", 'number', []);

        //video element
        var mediaSource;
        var url;
        var videoSource;
        var totalduration;

        function handleInitSeg() {

            persecondlen = getdatapersecond();
            console.log("per second data " + persecondlen);
            reqsize = persecondlen;

            recvlen = persecondlen / 10 * 12;
            recvbuf = Module._malloc(recvlen);

            initlen = getinitseg(recvbuf, 0);
            initlen *= -1;
            console.log("init seg len " + initlen);

            if (recvlen < initlen) {
                recvlen = initlen;
                Module._free(recvbuf)
                recvbuf = Module._malloc(recvlen);
            }

            if (initlen != getinitseg(recvbuf, recvlen)) {
                console.log("exception when get init seg");
                throw "exception when get init seg";
            }

            setVideo();
        }

        function handleDataSeg() {
            var n = getnormalSeg(recvbuf, 0);
            n *= -1;
            //console.log("n s len ", n);

            if (n == 0) {
                console.error("getnormalSeg 0");
                return;
            }

            if (recvlen < n) {
                recvlen = n;
                Module._free(recvbuf)
                recvbuf = Module._malloc(recvlen);
                console.log("remalloc recvbuf");
            }

            if (n != getnormalSeg(recvbuf, recvlen)) {
                console.error("exception when get normal seg");
                throw "exception when get normal seg";
            }

            totalduration += getnormalSegDuration();
            //console.log("total duration " + totalduration);
            try {
                videoSource.appendBuffer(Module.HEAP8.subarray(recvbuf, recvbuf + n));
            } catch (err) {
                console.warn("ssssss");
                console.warn("ddddddd");

                console.log(err);
                console.warn(err.name);
                console.warn(err.message);
                console.warn(err.lineNumber);
                console.warn(err.columnNumber);
                console.warn(err.stack);
                console.warn(err.code);
            }
        }

        function checkFlvGet() {
            console.log("checkFlvGet");
            addVideoDataToDashConvert(new Int8Array(), 0);
        }

        function checkTimeForPreload(e) {
            //console.log("current time " + e.target.currentTime);
            //var lefttime = totalduration - e.target.currentTime;
            //console.log("lefttime of current seg " + lefttime);
            //if (lefttime < 5.0) {
            //    checkFlvGet();
            //}
        }

        function onSourceBufferUpdateStart(e) {
            //console.log(e);
            //console.log("onSourceBufferUpdateStart");
            //console.log(e.target.updating);
        }
        function onSourceBufferUpdateEnd(e) {
            //console.log("onSourceBufferUpdateEnd");
            //console.log(e.target.updating);

            //var s, e;
            //for (var i = 0; i < videoSource.buffered.length; i++) {
            //    s = videoSource.buffered.start(i)//第i个range信息的开始时间 
            //    e = videoSource.buffered.end(i);//第i个range信息的结束时间 
            //    console.warn(s);
            //    console.error(e);
            //}

            setTimeout(function () {
                checkFlvGet();
            }, 100);
        }
        function onSourceBufferUpdate(e) {
            //console.log(e);
            //console.log("onSourceBufferUpdate");
            //console.log(e.target.updating);

        }

        function onVideoSeeking(e) {
            console.warn(e);
            console.log(video_element.currentTime);
        }

        function onVideoSeeked(e) {
            console.warn(e);
            console.log(video_element.currentTime);

        }

        function setVideo() {
            console.log("setup video mse");

            try {
                mediaSource = new MediaSource;
                url = URL.createObjectURL(mediaSource);
            } catch (err) {
                console.log(err);
                throw "create media source error";
                return;
            }

            video_element.pause();
            video_element.src = url;

            mediaSource.addEventListener("sourceopen", function (e) {
                console.log("source open");

                try {
                    videoSource = mediaSource.addSourceBuffer('video/mp4; codecs="avc1.42E01E, mp4a.40.2"');
                } catch (err) {
                    console.error(err);
                    throw "add video source error";
                    return;
                }

                video_element.addEventListener("pause", function (e) {
                    console.log("video pause");
                });

                video_element.addEventListener("playing", function (e) {
                    console.log("video playing");
                });

                video_element.addEventListener("ended", function (e) {
                    console.log("video ended");
                    video_element.removeEventListener("timeupdate", checkTimeForPreload);
                });

                video_element.addEventListener("timeupdate", checkTimeForPreload);

                videoSource.addEventListener("updateend", onSourceBufferUpdateEnd);
                videoSource.addEventListener("updatestart", onSourceBufferUpdateStart);
                videoSource.addEventListener("update", onSourceBufferUpdate);
                try{
                    videoSource.appendBuffer(new Int8Array(Module.HEAP8.subarray(recvbuf, recvbuf + initlen)));
                } catch (err) {
                    console.warn("setupvideo add first buffer");
                    console.error(err);
                }
            });
        }

        function reqMoreVideoData() {
            //console.log("req more data");
            if (rangestartpos + reqsize <= videosize) {
                reqVideo(rangestartpos, rangestartpos + reqsize - 1);
            } else {

                if (endoffile) {
                    return;
                }

                console.log("almost reach end of file");
                reqsize = videosize - rangestartpos;

                if (reqsize > 0) {
                    console.log(reqsize + " bytes data left");
                    reqVideo(rangestartpos, rangestartpos + reqsize - 1);
                } else {
                    console.warn("end of file");
                    endoffile = true;
                    reqsize = 10000;

                    console.log("try get last seg");
                    getnormalSeg = getlastnormalSeg;
                    handleDataSeg();
                }
            }
        }

        function reqVideo(st, en) {
            //console.log("req " + st + " - " + en);
            try{           
                xhr = null;
                xhr = new XMLHttpRequest();
                xhr.addEventListener("readystatechange", OnXHRStateChanged);
                xhr.open("GET", video_url);
                xhr.responseType = "arraybuffer";
                xhr.setRequestHeader("Range", "bytes=" + st + "-" + en);
                xhr.send();
            } catch (err) {
                console.error(err);
                xhr.removeEventListener("readystatechange", OnXHRStateChanged);
                xhr = null;
            }
           
        }

        function OnXHRStateChanged(e) {

            if (xhr.readyState == XMLHttpRequest.DONE) {
                //console.log(e.target.getAllResponseHeaders());

                rangestartpos += reqsize;
                addVideoDataToDashConvert(new Int8Array(xhr.response), xhr.response.byteLength);
                if (videosize == 0) {
                    videosize = parseInt(e.target.getResponseHeader("content-range").split('/')[1]);
                }
            }
        }

        function addVideoDataToDashConvert(buf, len) {

            var n = addbuffer(buf, len);

            switch (n) {
                case 3:
                    //console.log("need more data");
                    reqMoreVideoData();
                    break;
                case 5:
                    //console.log("got normal seg");
                    handleDataSeg();
                    break;
                case 4:
                    console.log("got init seg");
                    handleInitSeg();
                    break;
                default:
                    console.error("error happened " + n);
                    break;
            }
        }

        this.play = function () {
            rangestartpos = 0;
            videosize = 0;
            initlen = 0;
            persecondlen = 0;
            reqsize = 409600;
            recvlen = 0;
            totalduration = 0.0;

            video_element.addEventListener("seeking", onVideoSeeking);
            video_element.addEventListener("seeked", onVideoSeeked);

            reqVideo(rangestartpos, rangestartpos + reqsize - 1);
        }
    }

    var obj = new ViewBoxPlayer(para);
    return obj;
}
