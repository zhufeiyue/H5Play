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

    return __vs_a[3]();
}


Module["createViewBoxPlayer"] = function (para) {

    function FileIOXHR(u, s, e, f) {
        this.url = u;
        this.reqs = s;
        this.reqe = e;
        this.cb = f;
        this.xhr = new XMLHttpRequest();
        this.xhr.addEventListener("readystatechange", OnXHRStateChanged);
        this.xhr.addEventListener("timeout", OnXHRTimeOut);
        this.xhr.addEventListener("error", OnXHRError);
        //this.xhr.addEventListener("progress", OnXHRProgress);
        this.xhr._cb0 = this.cb;
        this.xhr._rst = 0; // request start time
        this.xhr._rd = 0;  // request duration
        this.xhrworking = false;

        function OnXHRStateChanged(e) {
            //var xhr = e.target;
            //if (xhr.readyState == 4) {
            //    if (xhr.status == 206) {
            //        xhr._rd = (new Date()).getTime() - xhr._rst;
            //        xhr._cb0(0, new Int8Array(xhr.response), xhr.response.byteLength);
            //    } else {
            //        xhr._cb0(1, xhr.status);
            //    }
            //}
            __vs_a[10](e);
        }

        function OnXHRTimeOut(e) {
            e.target._cb0(2);
        }

        function OnXHRError(e) {
            e.target._cb0(3, e.toString());
        }

        function OnXHRProgress(e) {

            if (e.lengthComputable && e.loaded == e.total) {
                var xhr = e.target;
                var speed = e.loaded * 1000.0 / ((new Date()).getTime() - xhr._rst);
                console.warn(speed/1024.0 + " kB per second");
            }

        }
    }
    FileIOXHR.prototype.request = function () {
        try {
            this.xhr.open("GET", this.url);
            this.xhr.responseType = "arraybuffer";
            this.xhr.setRequestHeader("Range", "bytes=" + this.reqs + "-" + this.reqe);
            this.xhr.setRequestHeader("Cache-Control", "max-age=864000");
            if (typeof window.m_vst2 == 'string') {
                this.xhr.setRequestHeader('Authorization', window.m_vst2);
            }
            //xhr.timeout = 20000;
            this.xhr._rst = (new Date()).getTime();
            this.xhr.send();
            return true;
        } catch (err) {
            console.warn(err);
            return false;
        }
    }
    FileIOXHR.prototype.abort = function () {
        //console.warn("xhr abort");
        this.xhr.abort();
        this.xhr = null;
    }
    FileIOXHR.prototype.getFileSize = function () {
        return parseInt(this.xhr.getResponseHeader("content-range").split('/')[1])
    }
    FileIOXHR.prototype.getReqTime = function () {
        return this.xhr._rd;
    }

    function VideoDataCache(n, d) {
        this.m_index = n;
        this.m_data = d;
    }
    VideoDataCache.prototype.clean = function () {
        this.m_index = -1;
        this.m_data = null;
    }

    function VideoDataCacheManager(n) {
        this.m_aa = new Array();
        this.m_count = n;
    }
    VideoDataCacheManager.prototype.addItem = function (i) {
        if (!this.isItemIn(i)) {
            this.m_aa.push(i);
            return true;
        }
        return false;
    }
    VideoDataCacheManager.prototype.delItem = function (i) {
        var n = -1;
        if (this.m_aa.some(function (item, index) {
            if (item.m_index == i.m_index) {
                n = index;
                return true;
                } else {
                return false;
        }})
            ) {
            this.m_aa.splice(n, 1);
        }
    }
    VideoDataCacheManager.prototype.delItemByIndex = function (i) {
        if (i >= 0 && i < this.m_aa.length) {
            this.m_aa.splice(i, 1);
        }
    }
    VideoDataCacheManager.prototype.isItemIn = function (i) {
        var res = -1;
        if (this.m_aa.length > this.m_count) {
            return res;
        }

        if (typeof i == "object") {
            this.m_aa.filter(function (item, index) {
                if (item.m_index == i.m_index) {
                    res = index;
                    return true;
                }
                return false;
            });
        } else {
            this.m_aa.filter(function (item, index) {
                if (item.m_index == i) {
                    res = index;
                    return true;
                }
                return false;
            });
        }

        return res;
    }

    function ViewBoxPlayer(pp) {

        window.m_vst2 = pp.t;
        var video_element = pp.wrap;
        //var video_url = "http://" + location.host + pp.videouri;
        var video_url = Module.ccall("_Z2aePc", 'string', ['string'], [pp.videouri]);
        var video_poster = video_url.replace(pp.videouri, pp.videoposter);
        video_element.poster = video_poster;
        var skin = Module.createViewBoxPlayerSkin(pp);
        skin.videoposterurl = video_poster;
        skin.videoposter = null;
        skin.init();
        var videosize;
        var reqsize;
        var rangestartpos;
        var endoffile = false;
        var xhr;
        var initlen;
        var normalseglen;
        var persecondlen;
        var recvbuf;
        var recvlen;
        var indexnumber;
        var indexinfo;
        var currentindex;
        var currentsegreqpos;
        var currentsegreqlen;
        var addbuffer = __vs_a[8];
        var addbufferref = __vs_a[9];
        var getinitseg = __vs_a[6];
        var getnormalSeg = __vs_a[7];
        //var getlastnormalSeg = Module.cwrap("_Z16GetLastNormalSegPci", 'number', ['number', 'number']);
        var getnormalSegDuration = Module.cwrap("_Z2aKv", 'number', []);
        var getdatapersecond = Module.cwrap("_Z3a__v", 'number', []);
        var getsegnumber = Module.cwrap("_Z2a_v", 'number', []);
        var getsegposlen = Module.cwrap("_Z2adi", 'string', ['number']);
        var gettotalduration = Module.cwrap("_Z2afv", 'double', []);
        var updatetr = Module.cwrap("_Z2agPfi", 'number', ['array', 'number']);
        var issegin = Module.cwrap("_Z2ahi", 'number', ['number']);
        var getsegbytime = Module.cwrap("_Z2aid", 'number', ['number']);
        var seekbysegindex = Module.cwrap("_Z2aji", null, ['number']);
        var clean = Module.cwrap("_Z2bbv", null, []);

        //video element
        var mediaSource;
        var url;
        var videoSource;
        var bufferfull = false;
        var totalduration;
        var bplaying;
        var preloadtime;
        var tid;
        var playtimeoutoftimerange;

        function handleInitSeg() {

            if (videosize == 0) {
                videosize = xhr.getFileSize();
                //console.log("file size is " + videosize);
            }
            totalduration = gettotalduration();
            //console.log("file duration is " + totalduration);
            preloadtime = totalduration * 5 / 100 + Math.random() * 10;
            //preloadtime = totalduration;
            if (preloadtime < 8) {
                preloadtime = 8;
            }
            if (totalduration < 300) {
                preloadtime += Math.random() * 30;
            }
            //console.log("file preload time is " + preloadtime);

            indexnumber = getsegnumber(/*Date.parse(xhr.getResponseHeader("date")) / 1000*/);
            //console.log("total seg number is " + indexnumber);

            currentindex = 0;

            persecondlen = getdatapersecond();
            //console.log("per second data " + persecondlen);
            reqsize = persecondlen;

            recvlen = persecondlen / 10 * 12;
            recvbuf = Module._malloc(recvlen);

            initlen = getinitseg(recvbuf, 0);
            initlen *= -1;
            //console.log("init seg len " + initlen);

            if (recvlen < initlen) {
                recvlen = initlen;
                Module._free(recvbuf)
                recvbuf = Module._malloc(recvlen);
            }

            if (initlen != getinitseg(recvbuf, recvlen)) {
                throw "exception when get init seg";
            }

            setVideo();
        }

        function handleDataSeg() {
            var n = getnormalSeg(recvbuf, 0);
            n *= -1;
            //console.log(currentindex+" n s len ", n);

            if (n == 0) {
                console.error("getnormalSeg 0");
                return;
            }

            if (recvlen < n) {
                recvlen = n;
                Module._free(recvbuf)
                recvbuf = Module._malloc(recvlen);
                //console.log("remalloc recvbuf");
            }

            if (n != getnormalSeg(recvbuf, recvlen)) {
                console.error("exception when get normal seg");
                throw "exception when get normal seg";
            }

            try {
                __vs_a[1](videoSource, Module.HEAP8.subarray(recvbuf, recvbuf + n));
            } catch (err) {
                if (err.code == 22) { // QuotaExceededError
                    bufferfull = true;
                    normalseglen = n;
                    console.warn("buffer is full");
                    clearTimeout(tid);
                    tid = setTimeout(function () {
                        checkForPreload();
                    }, 800);
                } else {
                    console.log(err);
                    console.warn(err.name);
                    console.warn(err.message);
                    console.warn(err.stack);
                    console.warn(err.code);
                    throw err;
                }
            }
        }

        function checkFlvGet() {
            //console.log("checkFlvGet");

            if (xhr.xhrworking) {
                clearTimeout(tid);
                tid = setTimeout(function () {
                    checkForPreload();
                }, 800);
                console.error("xhrworking true");
                return;
            }

            if (videoSource.updating) {
                console.warn('buffer updating');
                return;
            }

            if (bufferfull) {
                try {
                    __vs_a[1](videoSource, Module.HEAP8.subarray(recvbuf, recvbuf + normalseglen));
                } catch (err) {
                    if (err.code == 22) { // QuotaExceededError
                        console.warn("buffer still full");
                        clearTimeout(tid);
                        tid = setTimeout(function () {
                            checkForPreload();
                        }, 8000);
                        return;
                    } else {
                        throw err;
                    }
                }

                console.log('buffer not full anymore');
                bufferfull = false;
                return;
            }
            
            if (currentindex + 1 < indexnumber) {
                currentindex += 1;
                var i = issegin(currentindex);
                // usefull log console.log(currentindex + " seg in " + i);
                if (i > 0) {
                    currentindex = i - 1;
                    clearTimeout(tid);
                    tid = setTimeout(function () {
                        checkForPreload();
                    }, 800);
                    return;
                }

                seekbysegindex(currentindex);

                currentsegreqpos = getsegposlen(currentindex).split('/');
                currentsegreqlen = parseInt(currentsegreqpos[1]);
                currentsegreqpos = parseInt(currentsegreqpos[0]);
                //console.log("seg " + currentindex + " req pos is " + currentsegreqpos + " req len is " + currentsegreqlen);

                if (currentsegreqlen > 0) {
                    reqsize = currentsegreqlen;
                    rangestartpos = currentsegreqpos;

                    // 4096*n
                    var end = rangestartpos + reqsize;
                    var v = end % 4096;
                    if (v > 0) {
                        v = 4096 - v;
                        if (end + v < videosize) {
                            reqsize += v;
                        }
                    }
                    reqVideo(currentsegreqpos, rangestartpos + reqsize - 1);
                } else {
                    addVideoDataToDashConvert(new Int8Array(), 0);
                }
            }
        }

        function checkForPreload() {
            var t = video_element.currentTime;
            var s, e, i;

            for (i = 0; i < videoSource.buffered.length; ++i) {
                s = videoSource.buffered.start(i)//第i个range信息的开始时间 
                e = videoSource.buffered.end(i);//第i个range信息的结束时间 
                //console.warn(s);
                //console.warn(e);

                if (t >= s && t <= e) {
                    if (e - t < preloadtime) {
                        //console.log("left time of this time range " + (e - t));
                        checkFlvGet();
                    } else {
                        if (xhr.speed < 1.0 && (e - t < preloadtime / xhr.speed)) {
                            checkFlvGet();
                        } else {
                            // usefull log console.log("checkForPreload on agenda");
                            clearTimeout(tid);
                            tid = setTimeout(checkForPreload, 3886);
                        }
                    }
                    return;
                }
            }

            console.warn("t out of tr " + videoSource.buffered.start(0));

            if (playtimeoutoftimerange < 10) {
                playtimeoutoftimerange++;
                checkFlvGet();
                return;
            }

            if (bplaying) {
                checkFlvGet();
            } else {
                console.log("checkForPreload on agenda f");
                clearTimeout(tid);
                tid = setTimeout(checkForPreload, 1000);
            }
        }

        function checkTimeForPreload(e) {

            //console.log("on timeupdate");
            //if (video_element.currentTime - lastplaytime > 1) {
            //    checkForPreload();
            //    lastplaytime = video_element.currentTime;
            //}
        }

        function onSourceBufferAbort(e) {
            console.log("source buffer update abort");
        }
        function onSourceBufferUpdateStart(e) {
            //console.log(e);
            //console.log("onSourceBufferUpdateStart");
            //console.log(e.target.updating);
        }
        function onSourceBufferUpdate(e) {
            //console.log(e);
            //console.log("onSourceBufferUpdate");
            //console.log(e.target.updating);

        }
        function onSourceBufferInitSegUpdateEnd(e) {

            videoSource.removeEventListener("updateend", onSourceBufferInitSegUpdateEnd);
            videoSource.addEventListener("updateend", onSourceBufferUpdateEnd);
            //console.log("d " + mediaSource.duration);
            skin.notifyVideo({
                type: 'init',
                duration: gettotalduration()
            });

            console.log("init seg update end; dir");
            checkFlvGet();
        }
        function onSourceBufferUpdateEnd(e) {

            var tr = new Float32Array(videoSource.buffered.length * 2);
            for (var i = 0; i < videoSource.buffered.length; ++i) {
                tr[i * 2] = videoSource.buffered.start(i);
                tr[i * 2 + 1] = videoSource.buffered.end(i);
            }
            updatetr(new Int8Array(tr.buffer), videoSource.buffered.length * 2);
            //console.log(tr);
            //tr = null;

            skin.notifyVideo({
                type: 'update',
                tr: tr
            });

            //if (currentindex == indexnumber - 1) {
            //    mediaSource.endOfStream();
            //    return;
            //}

            clearTimeout(tid);
            tid = setTimeout(function () {
                checkForPreload();
            }, 100);
        }
        
        function onVideoSeeking(e) {
            //console.warn(e);
            //console.log("seek time is " + video_element.currentTime);
            var i = getsegbytime(video_element.currentTime);
            //console.warn("seek index is " + i);
            clearTimeout(tid);

            if (xhr.xhrworking) {
                xhr.abort();
            }
            if (videoSource.updating) {
                videoSource.abort();
            }

            tid = setTimeout(function () {
                currentindex = i - 1;
                playtimeoutoftimerange = 0;
                bufferfull = false;
                checkForPreload();
            }, 200);
        }

        function onVideoSeeked(e) {
            console.warn(e);
            console.log(video_element.currentTime);
        }

        function setVideo() {
            //console.log("setup video mse");

            try {
                mediaSource = __vs_a[0]();
                url = URL.createObjectURL(mediaSource);
            } catch (err) {
                console.log(err);
                throw "create media source error";
                return;
            }

            video_element.pause();
            video_element.src = url;
            bplaying = false;

            mediaSource.addEventListener("sourceopen", function (e) {
                console.log("source open");

                try {

                    videoSource = mediaSource.addSourceBuffer(__vs_a[2]);
                    //videoSource = mediaSource.addSourceBuffer('video/mp4');
                    playtimeoutoftimerange = 0;
                    //console.log(videoSource);

                } catch (err) {
                    console.error(err);
                    throw "add video source error";
                    return;
                }

                video_element.addEventListener("pause", function (e) {
                    bplaying = false;
                    //console.warn("pause time is " + video_element.currentTime);
                    skin.notifyVideo({
                        type: 'pause'
                    });
                });

                video_element.addEventListener("playing", function (e) {
                    bplaying = true;
                    //console.warn("play time is " + video_element.currentTime);
                    skin.notifyVideo({
                        type: 'play'
                    });
                });

                video_element.addEventListener("ended", function (e) {
                    console.log("video ended");
                    video_element.removeEventListener("timeupdate", checkTimeForPreload);
                });

                skin.notifyVideo({
                    type: 'checksarforwebkit'
                });

                videoSource.addEventListener("updateend", onSourceBufferInitSegUpdateEnd);
                videoSource.addEventListener("updatestart", onSourceBufferUpdateStart);
                //videoSource.addEventListener("update", onSourceBufferUpdate);
                videoSource.addEventListener("abort", onSourceBufferAbort);
                try{
                    __vs_a[1](videoSource, new Int8Array(Module.HEAP8.subarray(recvbuf, recvbuf + initlen)));
                } catch (err) {
                    console.error(err);
                }
            });
            mediaSource.addEventListener("sourceended", function (e) {
                console.log("source ended");
            });
            mediaSource.addEventListener("sourceclose", function (e) {
                console.log("source close");
            });
        }

        function reqMoreVideoData() {
            //console.log("req more data");
            if (rangestartpos + reqsize <= videosize) {
                reqVideo(rangestartpos, rangestartpos + reqsize - 1);
            } else {
                console.error("request range out of file");
            }
        }

        function reqVideo(st, en) {
            //console.log("req " + st + " - " + en);
            xhr = null;
            xhr = new FileIOXHR(video_url, st, en, onXHRCB);
            if (!xhr.request()) {
                console.log("get file error");
                xhr.xhrworking = false;
            } else {
                xhr.xhrworking = true;
            }
        }

        function onXHRCB(n, s, a) {
            if (n == 0) {
                if (a == 0) {
                    console.log("abort???");
                    xhr.xhrworking = false;
                    return;
                }
                if (a != reqsize) console.warn("response short ");
                rangestartpos += a;
                reqsize -= a;
                addVideoDataToDashConvert(s, a);
                xhr.speed = getnormalSegDuration() * 1000.0 / xhr.getReqTime();
                // usefull log console.log("down speed is " + xhr.speed);
            } else if (n == 1) {
                console.warn("need 206 " + s);
            } else if (n == 2) {
                console.warn("request timeout");
            } else if (n == 3) {
                console.error(s);
            }
            xhr.xhrworking = false;
        }

        function addVideoDataToDashConvert(buf, len) {

            var n = 0;
            // emcripten stack size limit
            if (len < 5 * 1024 * 1024) {
                n = addbuffer(buf, len);
            } else {
                t = Module._malloc(len)
                Module.HEAP8.set(buf, t);
                n = addbufferref(t, len);
                Module._free(t)
            }

            switch (n) {
                case 5:
                    //console.log("got normal seg");
                    handleDataSeg();
                    break;
                case 4:
                    //console.log("got init seg");
                    handleInitSeg();
                    break;
                case 3:
                    console.log("need more data");
                    reqMoreVideoData();
                    break;
                default:
                    console.error("error happened " + n);
                    break;
            }
        }

        this.play = function () {
            rangestartpos = 0;
            videosize = 409600;
            initlen = 0;
            persecondlen = 0;
            reqsize = 409600;
            recvlen = 0;
            totalduration = 0.0;

            video_element.addEventListener("seeking", onVideoSeeking);
            video_element.addEventListener("seeked", onVideoSeeked);

            console.warn(video_url);
            reqMoreVideoData();
            videosize = 0;
        }

        this.stop = function () {
            try{
                video_element.pause();
                recvlen = 0;
                Module._free(recvbuf)
                indexnumber = 0;
                currentindex = 0;
                clean();
                skin.clean();
                xhr.abort();
                clearTimeout(tid);
                if (videoSource != null) {
                    videoSource.removeEventListener("updateend", onSourceBufferUpdateEnd);
                    videoSource.abort();
                    mediaSource.removeSourceBuffer(videoSource);
                    videoSource = null;
                }
                if (mediaSource != null){
                    mediaSource.endOfStream();
                    mediaSource = null;
                }
                url = null;
                video_element.src = null;
            } catch (err) {
                console.error(err);
            }
        }
    }

    var obj = new ViewBoxPlayer(para);
    return obj;
}

window.Module = Module;