
// Copyright  2018  Tim Niemueller <niemueller@kbsg.rwth-aachen.de>
// License: Apache 2.0 (http://www.apache.org/licenses/LICENSE-2.0)

import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ServiceWorkerModule } from '@angular/service-worker';
import { HttpClientModule } from '@angular/common/http';

import { environment } from '../environments/environment';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { ChromeModule } from '../chrome/module';
import { ConfigurationService } from '../services/config.service';
import { AssetsService } from '../services/global/assets';

import { ClipsExecutiveModule } from '../parts/clips-executive/module';
import { SkillerModule } from '../parts/skiller/module';

import { HttpClientInMemoryWebApiModule } from 'angular-in-memory-web-api';

@NgModule({
	declarations: [
	  AppComponent,
	],
	imports: [
	  BrowserModule,
	  BrowserAnimationsModule,
	  
	  HttpClientModule,
	  ServiceWorkerModule.register('/ngsw-worker.js', { enabled: environment.production }),

	  ChromeModule,
	  ClipsExecutiveModule,
	  SkillerModule,

	  // Keep the AppRoutingModule last
	  AppRoutingModule,
	],
  providers: [ConfigurationService, AssetsService],
	bootstrap: [AppComponent]
})
export class AppModule { }