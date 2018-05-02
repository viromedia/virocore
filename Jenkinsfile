pipeline {
  agent any
  stages {
    stage('clean') {
      steps {
        sh '''cd android
fastlane gradle_clean'''
      }
    }
    stage('virocore_lib') {
      steps {
        sh '''cd android
fastlane renderer_viro_core_lib'''
      }
    }
    stage('viro_react_lib') {
      steps {
        sh '''cd android
fastlane renderer_viro_react_lib'''
      }
    }
    stage('archive viroreact-release.aar') {
      steps {
        archiveArtifacts 'android/viroreact/build/outputs/aar/viroreact-release.aar'
      }
    }
  }
  environment {
    LC_ALL = 'en_US.UTF-8'
    LANG = 'en_US.UTF-8'
    ANDROID_HOME = '/Users/Shared/Jenkins/Library/Android/sdk'
  }
}