{
  "targets": [
    {
      "target_name": "ctp",
      "sources": [ "src/ctp.cc","src/tools.cc","src/stdafx.cpp","src/uv_mduser.cpp","src/uv_trader.cpp","src/wrap_mduser.cpp","src/wrap_trader.cpp" ],
      "libraries":["20160628_tradeapi64_windows/thostmduserapi.lib","20160628_tradeapi64_windows/thosttraderapi.lib"],
      "include_dirs":["20160628_tradeapi64_windows/", "src"]
    }
  ],
}