pipeline {
  agent any
  stages {
    stage('virocore_lib') {
      steps {
        sh '''cd android
fastlane renderer_viro_core_lib'''
      }
    }
  }
  environment {
    LC_ALL = 'en_US.UTF-8'
    LANG = 'en_US.UTF-8'
    ANDROID_HOME = '/Users/Shared/Jenkins/Library/Android/sdk'
  }
}