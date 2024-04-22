//
//  VisionPro13thCamApp.swift
//  VisionPro13thCam
//  
//  Created by Yos on 2024
//  
//

import SwiftUI

@main
struct VisionPro13thCamApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
				.opacity(0.6)
        }

        ImmersiveSpace(id: "ImmersiveSpace") {
            ImmersiveView()
        }
    }
}
