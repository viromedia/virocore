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
        sh '''cd android
fastlane renderer_cp_viro_react_lib_to_tmp'''
      }
    }
  stage('virokit_framework (ios)') {
      steps {
        sh '''cd ios
fastlane release_virokit_framework'''
      }
    }

    stage('releasetest') {
      steps {
        sh '''cd android
fastlane renderer_releasetest'''
      }
    }
    stage('memoryleaktest') {
      steps {
        sh '''cd android
fastlane renderer_memoryleaktest'''
      }
    }
    stage('start react-viro') {
      steps {
        build(job: 'react-viro/master', propagate: true, wait: true)
      }
    }
  }
  environment {
    LC_ALL = 'en_US.UTF-8'
    LANG = 'en_US.UTF-8'
  }
}