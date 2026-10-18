import asyncio, json, os, sys, time
os.environ["PATH"]=os.path.abspath("bin")+os.pathsep+os.environ["PATH"]
os.add_dll_directory(os.path.abspath("bin"))
import aiohttp, opuslib_next
async def main():
    pcm=open("_test/q.pcm","rb").read()
    enc=opuslib_next.Encoder(16000,1,opuslib_next.APPLICATION_VOIP); dec=opuslib_next.Decoder(24000,1)
    async with aiohttp.ClientSession() as s:
        r=await s.post("http://192.168.137.1:8003/xiaozhi/ota/",json={"application":{"version":"2.5.2"}},headers={"Device-Id":"aa:bb:cc:dd:ee:ff"})
        url=(await r.json())["websocket"]["url"]; print("OTA ->",url)
        async with s.ws_connect(url,headers={"Device-Id":"aa:bb:cc:dd:ee:ff","Protocol-Version":"1"}) as ws:
            await ws.send_str(json.dumps({"type":"hello","version":1,"transport":"websocket","audio_params":{"format":"opus","sample_rate":16000,"channels":1,"frame_duration":60}}))
            print("hello <-", (await ws.receive()).data)
            await ws.send_str(json.dumps({"type":"listen","state":"start","mode":"auto"}))
            got={"frames":0,"first":None}; t_end=[None]
            async def rx():
                async for m in ws:
                    if m.type==aiohttp.WSMsgType.BINARY:
                        dec.decode(m.data,1440); got["frames"]+=1
                        if got["first"] is None and t_end[0]: got["first"]=time.time()-t_end[0]
                    elif m.type==aiohttp.WSMsgType.TEXT:
                        d=json.loads(m.data); print("  <-",{k:v for k,v in d.items() if k!="session_id"})
                        if d.get("type")=="tts" and d.get("state")=="stop": return
            task=asyncio.create_task(rx())
            F=960*2
            for i in range(0,len(pcm),F):
                fr=pcm[i:i+F].ljust(F,b"\0"); await ws.send_bytes(enc.encode(fr,960)); await asyncio.sleep(0.06)
            t_end[0]=time.time()
            for _ in range(60):  # 3.6 s silence like a real mic
                if task.done(): break
                await ws.send_bytes(enc.encode(b"\0"*F,960)); await asyncio.sleep(0.06)
            await asyncio.wait_for(task,40)
            print(f"RESULT: first audio {got['first'] and round(got['first'],2)}s after end of speech | {got['frames']} frames = {got['frames']*0.06:.1f}s audio")
asyncio.run(main())
