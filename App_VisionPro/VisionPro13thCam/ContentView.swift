//
//  ContentView.swift
//  VisionCam13th
//
//  Created by Yos on 2024
//
//

import SwiftUI
import RealityKit
import RealityKitContent
import WebKit

#if os(macOS)
struct WebView: NSViewRepresentable {
	let loardUrl: URL
	func makeNSView(context: Context) -> WKWebView {
		return WKWebView()
	}
	func updateNSView(_ uiView: WKWebView, context: Context) {
		let request = URLRequest(url: loardUrl)
		uiView.load(request)
	}
}
#else
struct WebView: UIViewRepresentable {
	let loardUrl: URL
	func makeUIView(context: Context) -> WKWebView {
		return WKWebView()
	}
	func updateUIView(_ uiView: WKWebView, context: Context) {
		let request = URLRequest(url: loardUrl)
		uiView.load(request)
	}
}
#endif


struct ContentView: View {
	var body: some View {
		WebView(loardUrl: URL(string: "http://192.168.10.123/stream")!)
	}
}

#Preview(windowStyle: .volumetric) {
	ContentView()
}

